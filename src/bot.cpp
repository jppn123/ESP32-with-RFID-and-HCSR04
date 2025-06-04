#include <Arduino.h>
#include "bot.h"
#include "wifiCred.h"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
unsigned long lastTimeBotRan;

String obterUid(String text, String metodo){
  int index = text.indexOf(metodo) + String(metodo).length() + 1;
  String uid = text.substring(index, text.length());
  return uid;
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
        welcome += "Comandos referente ao controle de usuários:\n";
        welcome += "/cadastrar {UID} para conceder acesso a um usuário\n";
        welcome += "/remover {UID} para remover acesso de um usuário\n";
        welcome += "/verificar para obter o UID\n";

        welcome += "\nComandos referente ao controle da caixa de água:\n";
        welcome += "/verificar para verificar a medição atual da caixa de água\n";
        
        welcome += "\nComandos de informação:\n";  
        welcome += "/info para visualizar todos os comandos possíveis novamente\n";
        welcome += "/log para visualizar os comandos de logs\n";

        bot.sendMessage(chat_id, welcome, "");
    }

    if (strstr(text.c_str(), "cadastrar")) {
      String uid = obterUid(text, "cadastrar");
      //TODO implementar a logica de inserir o cartao na lista de cartoes permitidos

      Serial.println("text: " + text);
      Serial.println("uid: " + uid);
      //bot.sendMessage(chat_id, "Usuário cadastrado com sucesso", "");
    }

    if (strstr(text.c_str(), "remover")) {
      String uid = obterUid(text, "remover");
      //TODO implementar a logica de remover o cartao da lista de cartoes permitidos
      
      Serial.println("text: " + text);
      Serial.println("uid: " + uid);
      //bot.sendMessage(chat_id, "Usuário removido com sucesso", "");
    }
    
    // if (text == "/on") {
    //   bot.sendMessage(chat_id, "Ligando LED", "");
    //   digitalWrite(LED, HIGH);
    // }
    
    // if (text == "/off") {
    //   bot.sendMessage(chat_id, "Desligando LED", "");
    //   digitalWrite(LED, LOW);
    // }
  }
}

void setupBot() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

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
