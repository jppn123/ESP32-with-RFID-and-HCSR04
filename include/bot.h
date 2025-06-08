#ifndef BOT_H
#define BOT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOTtoken "8105368493:AAENsDUCdCiR7kQc3flK6qZG-1OkZ0-ijRg"  
#define CHAT_ID "1546484802"
#define DELAY 500

void comunicateToBot();
void setupBot();
void handleNewMessages(int numNewMessages);

#endif
