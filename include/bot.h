#ifndef BOT_H
#define BOT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOTtoken "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"  
#define CHAT_ID "xxxxxxxxxxxxx"
#define DELAY 500

void comunicateToBot();
void setupBot();
void handleNewMessages(int numNewMessages);

#endif
