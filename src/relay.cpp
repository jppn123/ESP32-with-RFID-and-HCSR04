#include <arduino.h>
#include "relay.h"

#define RELE 27

void setupRelay(){
    pinMode(RELE, OUTPUT);
}
//state = 0 ou 1
void modifyRelay(bool state){
    digitalWrite(RELE, state); //rele ativa com LOW no vin
}
