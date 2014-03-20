#include <HardwareSerial.h>
#include "inttypes.h"
#include "string.h"
#include "LibXBee.h"
#include "utils.h"

XBeeSeries1 * myXBee;

long lastMessage = 0;

void setup() {
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  
  
  Serial.begin(9600);
  myXBee = new XBeeSeries1(&Serial);
  myXBee->init();
}


void loop() {
  myXBee->listen();
  
  if(checkTimeout(lastMessage,1000)) {
    myXBee->sendTo16(0xFFFF,"Hello World\r\n");
    lastMessage = millis();
  }
}





