#include <Arduino.h>
#include "rfid.h"
#include "sensor.h"
#include "relay.h"
#include "fsApp.h"
#include "bot.h"

#define LEDVERDE 13
#define LEDVERMELHO 17
//usado para quando o relé não estiver ligado
#define PINLOW 25

byte uid[10];
byte uidSize;
void sensorTask(void* parameter);
void resetAcessoTask(void* parameter);
void simulaAcesso(bool vermelhoLigado, bool verdeLigado, bool releLigado);

void setup() {
    Serial.begin(115200);
    
    setupRFID();
    setupSensor();
    setupRelay();
    setupFS();
    setupBot();


    pinMode(LEDVERDE, OUTPUT);
    pinMode(LEDVERMELHO, OUTPUT);
    pinMode(PINLOW, OUTPUT);

    //iniciar com acesso negado e rele desligado
    simulaAcesso(true, false, true);
    
    //separar a funcionalidade da caixa de agua do resto do sistema
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 1, NULL, 0);
}

void loop() {
    if (readRFID(uid, uidSize)) {
        String uidr = printUID(uid, uidSize);
        Serial.println(uidr);
        if(uidr == "43 75 3D 21"){
            Serial.println("autorizado");
            AddMessageToUsersLog("usuario autorizado");
            simulaAcesso(false, true, false);

            //remover o acesso após 20 segundos
            xTaskCreate(resetAcessoTask, "ResetAcesso", 2048, NULL, 1, NULL);
        }else{
            Serial.println("nao autorizado");
            AddMessageToUsersLog("usuario nao autorizado");
            simulaAcesso(true, false, true);
        }    
        delay(500);
    }
    comunicateToBot();
    //ReadUsersLog();
    //delay(1000);
    //ReadWaterLog();
}

//sensor distancia
void sensorTask(void* parameter) {
  while(true) {
    useSensorAndChangeLed();
    //delay de 30 minutos
    vTaskDelay(1800000 / portTICK_PERIOD_MS);  
  }
}

void resetAcessoTask(void* parameter) {
  vTaskDelay(20000 / portTICK_PERIOD_MS);

  simulaAcesso(true, false, true);

  vTaskDelete(NULL);
}

void simulaAcesso(bool vermelhoLigado, bool verdeLigado, bool releDesligado){
    digitalWrite(PINLOW, !releDesligado);
    digitalWrite(LEDVERMELHO, vermelhoLigado);
    digitalWrite(LEDVERDE, verdeLigado);
    //liga no 0 (false) desliga no 1 (true)
    modifyRelay(releDesligado);
}

