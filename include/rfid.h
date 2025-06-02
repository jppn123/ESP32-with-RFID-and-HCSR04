#ifndef RFID_H
#define RFID_H

#include <MFRC522.h>
#include <SPI.h>

void setupRFID();
bool readRFID(byte *uid, byte &uidSize);
String printUID(const byte *uid, byte uidSize);

#endif
