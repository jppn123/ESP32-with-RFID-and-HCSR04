#include <Arduino.h>
#include "rfid.h"

#define RST_PIN  21
#define SS_PIN   5

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setupRFID() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID pronto para uso");
}

bool readRFID(byte *uid, byte &uidSize) {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  uidSize = mfrc522.uid.size;
  for (byte i = 0; i < uidSize; i++) {
    uid[i] = mfrc522.uid.uidByte[i];
  }

  mfrc522.PICC_HaltA();
  return true;
}

String printUID(const byte *uid, byte uidSize) {
  String uidStr = "";
  for (byte i = 0; i < uidSize; i++) {
    uidStr += uid[i] < 0x10 ? " 0" : " ";
    uidStr += String(uid[i], HEX);
  }
  if(uidStr.substring(0,1) == " "){
    uidStr = uidStr.substring(1);
  } 
  uidStr.toUpperCase();  
  return uidStr;
}
