#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include <WiFi.h>
#include <time.h>
#include "fsApp.h"
#include "wifiCred.h"

#define FORMAT_LITTLEFS_IF_FAILED true
#define TZ "BRT3"

String getDateNow();
String getTimeNow();

String logPathUsers;
String logPathWater;
String logPathAcesso;
//fazer uma "cache" para não precisar consultar um arquivo sempre que for verificar se o usuário está autorizado
int flagModificouListaUsuariosAutorizados; 
std::vector<String> authorizedUsers;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    if(fs.exists(path)){
      Serial.printf("DIR %s already exists \n", path);
      return;
    }
    
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }       
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

std::vector<String> readFileLines(fs::FS &fs, const char *path) {
    std::vector<String> lines;

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- falha ao abrir o arquivo para leitura");
        return lines;
    }

    String currentLine = "";

    while (file.available()) {
        char c = file.read();
        if (c == '\n') {
            currentLine.trim();
            if (currentLine.length() > 0) {
                lines.push_back(currentLine);
            }
            currentLine = "";
        } else {
            currentLine += c;
        }
    }

    file.close();
    return lines;
}

std::vector<String> getAuthorizedUsers(fs::FS &fs) {
    std::vector<String> authorizedUsers;
    File file = fs.open(logPathAcesso.c_str());
    if (!file) {
        Serial.println("- failed to open file for reading");
        return authorizedUsers;
    }

    String currentLine = "";

    while (file.available()) {
        char c = file.read();
        if (c == '\n') {
            currentLine.trim(); // remove espaços e quebras extras
            if (currentLine.length() > 0) {
                authorizedUsers.push_back(currentLine);
            }
            currentLine = ""; // reinicia para a próxima linha
        } else {
            currentLine += c;
        }
    }

    file.close();

    return authorizedUsers; // Retorna o vetor de usuários autorizados
}

bool UserInAuthorizedList(String userToCheck) {
    if(flagModificouListaUsuariosAutorizados){
        // Se houve mudança, atualiza a lista de usuários autorizados
        authorizedUsers = getAuthorizedUsers(LittleFS);
        flagModificouListaUsuariosAutorizados = 0; // Reseta o flag
    }

    for (String user : authorizedUsers) {
        if (user == userToCheck) {
            return true;
        }
    }

    return false;
}

String removeAuthorizedUser(fs::FS &fs, String userToRemove) {
    File file = fs.open(logPathAcesso.c_str(), "r");
    if (!file) {
        Serial.println("- falha ao abrir o arquivo para leitura");
        return "Houve um erro de Leitura do arquivo";
    }

    std::vector<String> remainingUsers;
    bool userFound = false;
    String currentLine = "";

    // Lê linha por linha e verifica se é o usuário a ser removido
    while (file.available()) {
        char c = file.read();
        if (c == '\n') {
            currentLine.trim();
            if (currentLine.length() > 0) {
                if (currentLine != userToRemove) {
                    remainingUsers.push_back(currentLine);
                } else {
                    userFound = true; // encontramos o usuário, então não adicionamos de volta
                }
            }
            currentLine = "";
        } else {
            currentLine += c;
        }
    }

    file.close();

    // Se não encontrou o usuário, não reescreve o arquivo
    if (!userFound) {
        return "Usuário informado não está presente na lista de autorizados";
    }

    // Reescreve o arquivo com os usuários restantes
    file = fs.open(logPathAcesso.c_str(), "w");
    if (!file) {
        Serial.println("- falha ao abrir o arquivo para escrita");
        return "Erro ao salvar o novo arquivo";
    }

    for (const auto &user : remainingUsers) {
        file.println(user);
    }
    file.close();
    
    return "Usuário removido com sucesso";
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    if(fs.exists(path)){
      Serial.printf("file in path %s already exists \n", path);
      return;
    }
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

String fixLogMessage(const char* message) {
  std::string temp = getTimeNow().c_str();  // String do horário
  temp += " | ";
  temp += message;
  temp += "\n";

  return String(temp.c_str());  // Retorna uma String válida (do Arduino)
}

void appendFile(fs::FS &fs, const char * path, const char * message, int flagAddTime = 0){
    if(!fs.exists(path)){
      Serial.printf("file in path %s doesnt exists \n", path);
      return;
    }
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(!flagAddTime){
        if(file.print(fixLogMessage(message))){
            Serial.println("- message appended");
        } else {
            Serial.println("- append failed");
        }        
    }else{
        if(file.print(message)){
            Serial.println("- message appended");
        } else {
            Serial.println("- append failed");
        }
    }


    file.close();
}

String getTimeNow() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erro ao obter tempo";
  }

  char buffer[11];  // HH_MM_SS → 8 caracteres + '\0'
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  return String(buffer);
}

String getDateNow() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erro ao obter tempo";
  }
  char buffer[11];
  snprintf(buffer, sizeof(buffer), "%02d_%02d_%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  return String(buffer);
}

void AddMessageToUsersLog(const char * message){
    appendFile(LittleFS, logPathUsers.c_str(), message, 0);
}

void AddMessageToWaterLog(const char * message){
    appendFile(LittleFS, logPathWater.c_str(), message, 0);
}
void AddAuthorizedUser(const char * UID){
    std::string UIDConcat = UID;
    UIDConcat += "\n";
    flagModificouListaUsuariosAutorizados = 1;
    appendFile(LittleFS, logPathAcesso.c_str(),UIDConcat.c_str(), 1);
}

std::vector<String> ReadUsersLog(){
    return readFileLines(LittleFS, logPathUsers.c_str());
}

std::vector<String> ReadWaterLog(){
    return readFileLines(LittleFS, logPathWater.c_str());
}

std::vector<String> ReadAuthorizedUsers(){
    return getAuthorizedUsers(LittleFS);
}

String RemoveAuthUser(String userToRemove){
    flagModificouListaUsuariosAutorizados = 1;
    return removeAuthorizedUser(LittleFS, userToRemove);
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

void setupFS(){
    
    Serial.println();
    // Conecta ao Wi-Fi
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // Configura o tempo via NTP
    configTzTime(TZ, "pool.ntp.org", "time.nist.gov");

    // Aguarda a sincronização
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Aguardando tempo NTP...");
        delay(100);
    }
    
    logPathUsers = "/users/Log_usuario_" + getDateNow() + ".txt";
    logPathWater = "/water/Log_caixa_agua_" + getDateNow() + ".txt";
    logPathAcesso = "/acesso/cred/acessos_permitidos.txt";
    
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    // deleteFile(LittleFS, logPathAcesso.c_str());
    createDir(LittleFS, "/users");
    createDir(LittleFS, "/water");
    createDir(LittleFS, "/acesso");
    createDir(LittleFS, "/acesso/cred");
    writeFile(LittleFS, logPathUsers.c_str(), "");
    writeFile(LittleFS, logPathWater.c_str(), "");
    writeFile(LittleFS, logPathAcesso.c_str(), ""); 
    authorizedUsers = getAuthorizedUsers(LittleFS);
    // AddAuthorizedUser("N1 C2 M3 K4");
    // AddAuthorizedUser("A1 B2 C3 D4");
    
    // ReadWaterLog();
    // listDir(LittleFS, "/users", 0);
    // listDir(LittleFS, "/water", 0);

    // deleteFile(LittleFS, logPathWater.c_str());
    // deleteFile(LittleFS, logPathUsers.c_str());

    //getAuthorizedUsers(LittleFS);
}
