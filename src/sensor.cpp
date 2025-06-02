#include <Arduino.h>
#include "sensor.h"
#include "fsApp.h"

#define TRIGPIN 4
#define ECHOPIN 16
#define PINOLED 14
#define SOUND_SPEED 0.034
//talvez personalizar depois
#define VOLUMEAGUAALTO 10
#define VOLUMEAGUABAIXO 5

long duration;
float distanceCm;
int valor;

void setupSensor() {
    pinMode(TRIGPIN, OUTPUT); 
    pinMode(ECHOPIN, INPUT); 
    pinMode(PINOLED, OUTPUT);
    ledcAttachPin(PINOLED, 0);
    ledcSetup(0, 1000, 10);
}

void useSensorAndChangeLed(){
    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIGPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);
    
    duration = pulseIn(ECHOPIN, HIGH);
    
    distanceCm = duration * SOUND_SPEED/2;
    
    if(distanceCm >= VOLUMEAGUAALTO){
        String msg = "Volume de agua baixo | distancia: " + String(distanceCm);
        AddMessageToWaterLog(msg.c_str());
        valor = 85;
    }
    if(distanceCm < VOLUMEAGUAALTO &&  distanceCm >= VOLUMEAGUABAIXO){
        String msg = "Volume de agua medio | distancia: " + String(distanceCm);
        AddMessageToWaterLog(msg.c_str());
        valor = 170;
    }
    if(distanceCm < VOLUMEAGUABAIXO){
        String msg = "Volume de agua alto  | distancia: " + String(distanceCm);
        AddMessageToWaterLog(msg.c_str());
        valor = 255;
    }

    ledcWrite(0, valor);
    
    delay(500);
}
