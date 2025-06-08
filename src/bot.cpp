#include <Arduino.h>
#include <vector>
#include "bot.h"
#include "wifiCred.h"
#include "fsApp.h"
#include "rfid.h"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
unsigned long lastTimeBotRan;
byte id[10];
byte idSize;

String validarUid(String uid) {
  if (uid.length() != 11) {
    return "O {UID} deve estar no formato XX XX XX XX.";
  }
  return uid;
}

String obterUid(String text, String metodo){
  int index = text.indexOf(metodo) + String(metodo).length() + 1;
  String uid = text.substring(index, text.length());
  return validarUid(uid);
}

void handleNewMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start" || text == "/info") {
        String welcome = "Olá " + from_name + ",\n";
        welcome += "Use os comandos abaixo para controlar o sistema:\n\n";
        welcome += "Comandos referentes ao controle de usuários à distância:\n";
        welcome += "/cadastrar {UID} - Conceder acesso a um usuário usando o UID do cartão\n";
        welcome += "/remover {UID} - Remover acesso de um usuário usando o UID do cartão\n";

        welcome += "\nComandos referentes ao controle de usuários local:\n";
        welcome += "/cadastro_local - Cadastrar o cartão por aproximação\n";
        welcome += "/remoção_local - Remover o cartão por aproximação\n";
        welcome += "/obter_uid - Obter o UID aproximando um cartão válido\n";

        welcome += "\nComandos referentes ao controle da caixa de água:\n";
        welcome += "/medicao_atual - Verificar a medição atual da caixa de água\n";
        
        welcome += "\nComandos de informação:\n";  
        welcome += "/info - Visualizar todos os comandos principais novamente\n";
        welcome += "/log - Visualizar os comandos de logs\n";

        bot.sendMessage(chat_id, welcome, "");
    }

    if (text.startsWith("/cadastrar")) {
      String uid = obterUid(text, "cadastrar");
      if(uid.startsWith("O {UID}")) {
        bot.sendMessage(chat_id, uid, "");
        return;
      }

      AddAuthorizedUser(uid.c_str());

      bot.sendMessage(chat_id, "Usuário cadastrado com sucesso", "");
    }

    if (text.startsWith("/remover")) {
      String uid = obterUid(text, "remover");
      if(uid.startsWith("O {UID}")) {
        bot.sendMessage(chat_id, uid, "");
        return;
      }
      
      bot.sendMessage(chat_id, RemoveAuthUser(uid), "");
    }

    if (text.startsWith("/cadastro_local")) {
      bot.sendMessage(chat_id, "Aproxime o cartão do leitor", "");
      while(true){
        if (readRFID(id, idSize)) {
          String uidr = printUID(id, idSize);
          AddAuthorizedUser(uidr.c_str());
          bot.sendMessage(chat_id, "Usuário cadastrado com sucesso", "");
        } 
      }
    }

    if (text.startsWith("/remoção_local")) {
      bot.sendMessage(chat_id, "Aproxime o cartão do leitor", "");
      while(true){
        if (readRFID(id, idSize)) {
          String uidr = printUID(id, idSize);
          bot.sendMessage(chat_id, RemoveAuthUser(uidr), "");
          break;
        } 
      }
    }

    if (text.startsWith("/obter_uid")) {
      bot.sendMessage(chat_id, "Aproxime o cartão do leitor", "");
      while(true){
        if (readRFID(id, idSize)) {
          String uidr = printUID(id, idSize);
          bot.sendMessage(chat_id, "UID: " + uidr, "");
          break;
        } 
      }
    }

    if (text.startsWith("/medicao_atual")) {
      std::vector<String> logWater = ReadWaterLog();
      String response = "Medição atual da caixa de agua:\n";
      if (logWater.empty()) {
        response += "Nenhum log da caixa de agua encontrado.";
      } else {
        response += logWater.back() + "\n"; 
      }
      
      bot.sendMessage(chat_id, response, "");
    }
    
    if (text == "/log") {
      String logMessage = "Logs disponíveis:\n";
      logMessage += "/log_users - Visualizar logs de usuários\n";
      logMessage += "/log_water - Visualizar logs da caixa d'água\n";
      logMessage += "/users_auth - Visualizar usuarios autorizados\n";
      logMessage += "/info - Visualizar todos os comandos principais novamente\n";
      bot.sendMessage(chat_id, logMessage, "");

    }
    if (text == "/log_users") {
      std::vector<String> logUsers = ReadUsersLog();
      String response = "Logs de usuário do dia:\n";
      if (logUsers.empty()) {
        response += "Nenhum log de usuário encontrado.";
      } else {
        for (const String& user : logUsers) {
          response += user + "\n";
        }
      }
      
      bot.sendMessage(chat_id, response, "");
    }
    
    if (text == "/log_water") {
      std::vector<String> logWater = ReadWaterLog();
      String response = "Logs da caixa de agua do dia:\n";
      if (logWater.empty()) {
        response += "Nenhum log da caixa de agua encontrado.";
      } else {
        for (const String& user : logWater) {
          response += user + "\n";
        }
      }
      
      bot.sendMessage(chat_id, response, "");
    }

    if (text == "/users_auth") {
      std::vector<String> authorizedUsers = ReadAuthorizedUsers();
      String response = "Usuários autorizados:\n";
      if (authorizedUsers.empty()) {
        response += "Nenhum usuário autorizado encontrado.";
      } else {
        for (const String& user : authorizedUsers) {
          response += user + "\n";
        }
      }
      bot.sendMessage(chat_id, response, "");
    }
  }
}

void setupBot() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
}

void comunicateToBot() {
  if (millis() > lastTimeBotRan + DELAY)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
