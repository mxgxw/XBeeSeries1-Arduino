#include <HardwareSerial.h>
#include "inttypes.h"
#include "string.h"
#include "LibXBee.h"

XBeeSeries1 * myXBee;

long lastMessage = 0;

void setup() {
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  
  
  Serial.begin(9600);
  myXBee = new XBeeSeries1(&Serial);
  myXBee->onFrameReceived(xBeeFrameReceived);
  myXBee->init();
}

void loop() {
  myXBee->listen();
  
  if((millis()-lastMessage)>1000) {
    myXBee->sendTo16(0xFFFF,"Hello World\r\n");
    lastMessage = millis();
  }
}

void xBeeFrameReceived() {
  if(myXBee->rcvSize==0) {
    return;
  }
  Serial.print("Received frame type: 0x");
  Serial.print(myXBee->rcvBuffer[0],HEX);
  Serial.print("\n");
  int i;
  switch(myXBee->rcvBuffer[0]) {
    // Parse status
    case 0x89:
      Serial.print("Frame id: ");
      Serial.print(myXBee->rcvBuffer[1],HEX);
      Serial.print("\n");
      Serial.print("Delivery Status: ");
      switch(myXBee->rcvBuffer[2]) {
        case 0x00:
          Serial.print("Success\n");
          break;
        case 0x01:
          Serial.print("No ACK received\n");
          break;
        case 0x02:
          Serial.print("CCA failure\n");
          break;
        case 0x03:
          Serial.print("Purged\n");
          break;
        }
      break;
    case 0x81:
          // Process data from 16bit address
          // Process data from 64bit address
          for(int i=5; i < myXBee->rcvSize; i++) {
              Serial.write(myXBee->rcvBuff[i]);
          }
      break;
    case 0x80:
          // Process data from 64bit address
          for(int i=11; i < myXBee->rcvSize; i++) {
              Serial.write(myXBee->rcvBuff[i]);
          }
      break;
  }
}
