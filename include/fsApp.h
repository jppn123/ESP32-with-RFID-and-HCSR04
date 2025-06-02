#ifndef FSAPP_H
#define FSAPP_H

#include "FS.h"
#include <LittleFS.h>

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void AddMessageToUsersLog(const char * message);
void AddMessageToWaterLog(const char * message);
void ReadUsersLog();
void ReadWaterLog();
void setupFS();
String getDateNow();


#endif
