#ifndef BOT_H
#define BOT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOTtoken "7666849132:AAEQL6CJxW8S0LGOkcKexKIYfPjw2PYgUt8"  
#define CHAT_ID "5939240488"
#define DELAY 1000

void comunicateToBot();
void setupBot();
void handleNewMessages(int numNewMessages);

#endif
