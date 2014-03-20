/*************************************************************
Reaccion.net connectivity board XBee event-based abstraction layer.
Copyright (C) 2014  Mario Gomez/mxgxw < mario.gomez _at- teubi.co >

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**************************************************************/
#ifndef TeUbico_XBeeS1
#define TeUbico_XBeeS1

#define BUFFSIZE 128
#define WAIT_TIMEOUT 10000

#define WAIT_MODEM 0x00
#define COMMAND_MODE 0x01
#define API_MODE2 0x02

// Internal status values: 
#define RSP_WAIT 0x00
#define RSP_LMSB 0x01
#define RSP_LLSB 0x02
#define RSP_RDDATA 0x03

// Modem status variables
#define TX_STAT_SUCCESS 0
#define TX_STAT_NOACK 1
#define TX_STAT_CCAFAIL 2
#define TX_STAT_PURGED 3

class XBeeS1 {
public:
  XBeeS1(HardwareSerial *serial);
  bool init();
  void listen();
  uint8_t sendTo64(uint32_t addr_high, uint32_t addr_low, char* data);
  uint8_t sendTo16(uint16_t addr,char* data);
  void onFrameReceived(void (*handler)(uint8_t *dataFrame, uint16_t dataSize));
  void onTXStatus(void (*handler)(uint8_t seq, uint8_t code));
  void onReceiveData16(void (*handler)(uint16_t addr, uint8_t *data, uint16_t dataSize));
  void onReceiveData64(void (*handler)(uint32_t addr_high,uint32_t addr_low, uint8_t *data, uint16_t dataSize));
  void onReceiveData(void (*handler)(uint8_t *data, uint16_t dataSize));
private:  
  uint16_t rcvSize;
  uint8_t *rcvBuffer;
  uint8_t seq;
  uint8_t xBeeStatus;
  char *buffer;
  uint16_t buffPos;
  
  uint8_t d; // Temporary holder for the data
  bool responseFound; // Flag to stop reading
  uint8_t lMSB,lLSB; // Packet length (MSB,LSB)
  int packetSize; // Packet size built from the length
  int checkSum;
  uint32_t lastData;
  uint32_t lastSerialData;
  uint8_t readStatus;
  
  void (*frameReceivedHandler)(uint8_t *dataFrame, uint16_t dataSize);
  void (*rx16Handler)(uint16_t addr, uint8_t *data, uint16_t dataSize);
  void (*rx64Handler)(uint32_t addr_high,uint32_t addr_low, uint8_t *data, uint16_t dataSize);
  void (*rxHandler)(uint8_t *data, uint16_t dataSize);
  void (*txStatHandler)(uint8_t seq, uint8_t code);
  
  // Serial data processing functions
  bool waitFor(char *response, void (*command)());
  bool waitFor(char *response);
  
  void flush_buffer();
  void append_buffer(char c);
  
  void escapeAndWrite(uint8_t &data);
  
  HardwareSerial * _HardSerial;
  
  bool escapeNext;
  
  void processFrame();
};
#endif
