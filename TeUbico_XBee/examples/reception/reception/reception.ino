#include <teubico_XBeeS1.h>
#include <teubico_utils.h>

XBeeS1 * myXBee;

// Variable for pseudo-timer
long lastMessage = 0;

void setup() {
  
  Serial.begin(9600);
  myXBee = new XBeeS1(&Serial);
  myXBee->onFrameReceived(frameReceived);
  myXBee->onDataReceived16(dataReceived16);
  myXBee->onDataReceived64(dataReceived64);
  myXBee->init();
  
}

void loop() {
  // Listen for new frames
  myXBee->listen();
}

void frameReceived(uint8_t *data, uint16_t dataSiz) {
  Serial.write("Frame Received\r\n");
}

void dataReceived16(uint16_t addr, uint8_t *data, uint16_t dataSize) {
  for(int i=0; i<dataSize; i++) {
    Serial.write(data[i]);
  }
}

void dataReceived64(uint32_t addr_high,uint32_t addr_low, uint8_t *data, uint16_t dataSize) {
  for(int i=0; i<dataSize; i++) {
    Serial.write(data[i]);
  }
}

