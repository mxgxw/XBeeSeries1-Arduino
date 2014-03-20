#include <HardwareSerial.h>
#include "inttypes.h"
#include "string.h"
#include "teubico_XBeeS1.h"
#include "teubico_utils.h"

XBeeS1 * myXBee;

long lastMessage = 0;

void setup() {
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  
  
  Serial.begin(9600);
  myXBee = new XBeeS1(&Serial);
  myXBee->onTXStatus(txStatusReceived);
  myXBee->onReceiveData64(dataReceived64);
  myXBee->onReceiveData16(dataReceived16);
  myXBee->init();
  
}

void loop() {
  myXBee->listen();
  
  if(checkTimeout(lastMessage,1000)) {
    //myXBee->sendTo64(0x00,0xFFFF,"Hello World\r\n");
    lastMessage = millis();
  }
}

void txStatusReceived(uint8_t seq, uint8_t statusCode) {
  Serial.print("Sequence number: ");
  Serial.println(seq, HEX);
  Serial.print("Status: ");
  switch(statusCode) {
    case 0:
      Serial.println("Success");
      break;
    case 1:
      Serial.println("No ACK (Acknowledgement) received");
      break;
    case 2:
      Serial.println("CCA failure");
      break;
    case 3:
      Serial.println("Purged");
      break;
  }
}

void dataReceived64(uint32_t addr_high, uint32_t addr_low, uint8_t *data, uint16_t dataSize) {
  Serial.print("Data Received: ");
  for(int i=0;i<dataSize;i++) {
    Serial.write(data[i]);
  }
}

void dataReceived16(uint16_t addr, uint8_t *data, uint16_t dataSize) {
  Serial.print("Data Received: ");
  for(int i=0;i<dataSize;i++) {
    Serial.write(data[i]);
  }
}



