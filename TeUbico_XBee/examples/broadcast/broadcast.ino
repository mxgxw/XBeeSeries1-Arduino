#include <teubico_XBeeS1.h>
#include <teubico_utils.h>

XBeeS1 * myXBee;

// Variable for pseudo-timer
long lastMessage = 0;

void setup() {
  
  Serial.begin(9600);
  myXBee = new XBeeS1(&Serial);
  myXBee->onTXStatus(txStatusReceived);
  myXBee->init();
  
}

void loop() {
  // Listen for new frames
  myXBee->listen();
  
  // Sends a broadcast every second
  if(checkTimeout(lastMessage,1000)) {
    myXBee->sendTo16(0xFFFF,"Hello World\r\n");
    lastMessage = millis();
  }
}

void txStatusReceived(uint8_t seq, uint8_t statusCode) {
  // Process the status response
  Serial.print("Sequence number: ");
  Serial.println(seq, HEX);
  Serial.print("Status: ");
  
  // Prints the message status
  switch(statusCode) {
    case TX_STAT_SUCCESS:
      Serial.println("Success");
      break;
    case TX_STAT_NOACK:
      Serial.println("No ACK (Acknowledgement) received");
      break;
    case TX_STAT_CCAFAIL:
      Serial.println("CCA failure");
      break;
    case TX_STAT_PURGED:
      Serial.println("Purged");
      break;
  }
}
