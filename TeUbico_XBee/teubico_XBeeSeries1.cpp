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
#include <stdlib.h>
#include <Arduino.h>
#include "inttypes.h"

#include "teubico_XBeeSeries1.h"

char convBuffer[32];

XBeeSeries1::XBeeSeries1(HardwareSerial &serial,uint16_t my_addr) {
  this->self_addr = my_addr;
  this->seq = 0;
  this->rcvSize = 0;
  this->rcvBuffer = NULL;

  this->xBeeStatus = WAIT_MODEM;
  this->buffer = (char*)malloc(BUFFSIZE);
  this->buffPos = 0;

  this->d = '\0';

  this->responseFound = false;

  this->lMSB = 0;
  this->lLSB = 0;

  this->packetSize = 0;
  this->checkSum = 0;

  this->lastData = 0;
  this->lastSerialData = 0;

  this->readStatus = RSP_WAIT;

  this->frameReceivedHandler = NULL;
  this->rx16Handler = NULL;
  this->rx64Handler = NULL;
  this->rxHandler = NULL;
  
  this->_HardSerial = &serial;
  
  this->escapeNext = false;
  
  
  this->queued16 = false;
  this->queued64 = false;
  
  this->queuedAddrHigh = 0;
  this->queuedAddrLow = 0;
  this->queuedAddr = 0;
  this->queuedString64 = NULL;
  this->queuedString16 = NULL;
}

bool XBeeSeries1::init() {
  bool waitForInit = true;
  
  bool confirmATAP2 = false;
  bool confirmATCN = false;
  bool confirmSelfAddr = false;
  uint8_t conv_size = 0;
   
  while(waitForInit) {
    switch(this->xBeeStatus) {
      case WAIT_MODEM:
        this->_HardSerial->write("+++");
        if(this->waitFor("OK")) {
          this->xBeeStatus = COMMAND_MODE;
        }
        break;
      case COMMAND_MODE:
      
        this->_HardSerial->write("ATAP2\r\n");
        confirmATAP2 = this->waitFor("OK");
	this->_HardSerial->write("ATMY");
	conv_size = sprintf(convBuffer,"%x",this->self_addr);
	this->_HardSerial->write((uint8_t*)convBuffer, conv_size);
	this->_HardSerial->write("\r\n");
        confirmSelfAddr = this->waitFor("OK");
        this->_HardSerial->write("ATCN\r\n");
        confirmATCN = this->waitFor("OK");
        
        if(confirmATAP2 && confirmATCN && confirmSelfAddr) {
          waitForInit = false;
          this->xBeeStatus = API_MODE2;
        } else {
          this->xBeeStatus = WAIT_MODEM;
        }
        break;
    }
  }
  
  return (this->xBeeStatus==API_MODE2);
}

uint8_t XBeeSeries1::broadcast(char* data) {
  this->sendTo16(0xFFFF, data);
}

void XBeeSeries1::sendTo16(uint16_t addr, char* data) { 
  if(this->readStatus==RSP_WAIT) {
    this->_sendTo16(addr, data);
  } else {
    this->queued16 = true;
    this->queuedAddr = addr;
    this->queuedString16 = data;
  }
}

uint8_t XBeeSeries1::_sendTo16(uint16_t addr, char* data) { 
  // Process Frames only in API Mode 2
  if(this->xBeeStatus!=API_MODE2) {
    return 0;
  }
  
  uint16_t mSize = strlen(data)+5;
  
  uint32_t Sum = 0;
  byte chkSum = 0;
  
  uint8_t tmpData = 0;
  
  this->_HardSerial->write(0x7E);
  tmpData = (mSize & 0xFF00)>>8;
  this->escapeAndWrite(tmpData);
  tmpData = mSize & 0xFF;
  this->escapeAndWrite(tmpData);
  
  // API Identifier
  tmpData = 0x01;
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  
  // Frame sequence number
  if(this->seq==0xFF) {
    this->seq = 0x01;
  } else {
    this->seq += 1;
  }
  this->escapeAndWrite(seq);
  Sum += seq;
  
  // Address
  tmpData = (uint8_t)((addr & 0xFF00)>>8);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)(addr & 0xFF);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  
  // Options
  tmpData = 0x00;
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  
  for(int i=0;i<strlen(data);i++){
    tmpData = (uint8_t)data[i];
    this->escapeAndWrite(tmpData);
    Sum += tmpData;
  }
  
  chkSum = 0xFF - (Sum&0xFF);
  this->escapeAndWrite(chkSum);
  
  this->_HardSerial->flush();
  
  return this->seq;
}

void XBeeSeries1::sendTo64(uint32_t addr_high, uint32_t addr_low, char* data) { 
  if(this->readStatus==RSP_WAIT) {
    this->_sendTo64(addr_high,addr_low, data);
  } else {
    this->queued64 = true;
    this->queuedAddrHigh = addr_high;
    this->queuedAddrLow = addr_low;
    this->queuedString64 = data;
  }
}

uint8_t XBeeSeries1::_sendTo64(uint32_t addr_high, uint32_t addr_low, char* data) {
  
  // Process Frames only in API Mode 2
  if(this->xBeeStatus!=API_MODE2) {
    return 0;
  }
  
  uint16_t mSize = strlen(data)+11;
  
  uint32_t Sum = 0;
  byte chkSum = 0;
  
  uint8_t tmpData = 0;
  
  // Header:
  this->_HardSerial->write(0x7E);
  tmpData = (mSize & 0xFF00)>>8;
  this->escapeAndWrite(tmpData);
  tmpData = mSize & 0xFF;
  this->escapeAndWrite(tmpData);
  
  // API Identifier
  tmpData = 0x00;
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  
  
  if(this->seq==0xFF) {
    this->seq = 0x01;
  } else {
    this->seq += 1;
  }
  this->escapeAndWrite(seq);
  Sum += seq;
  
  // Address HIGH
  tmpData = (uint8_t)((addr_high & 0xFF000000)>>24);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)((addr_high & 0x00FF0000)>>16);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)((addr_high & 0x0000FF00)>>8);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)(addr_high & 0x000000FF);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  
  
  // Address LOW
  tmpData = (uint8_t)((addr_low & 0xFF000000)>>24);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)((addr_low & 0x00FF0000)>>16);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)((addr_low & 0x0000FF00)>>8);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
  tmpData = (uint8_t)(addr_low & 0x000000FF);
  this->escapeAndWrite(tmpData);
  Sum += tmpData;

  // Options
  tmpData = 0x00;
  this->escapeAndWrite(tmpData);
  Sum += tmpData;
    
  for(int i=0;i<strlen(data);i++){
    tmpData = (uint8_t)data[i];
    this->escapeAndWrite(tmpData);
    Sum += tmpData;
  }
  
  chkSum = 0xFF - (Sum&0xFF);
  this->escapeAndWrite(chkSum);
  
  this->_HardSerial->flush();
  return this->seq;
}

void XBeeSeries1::listen() {
  if(this->_HardSerial->available() && (this->xBeeStatus==API_MODE2)) {
    this->d = this->_HardSerial->read();
    
    if(this->d==0x7D) {
      this->escapeNext = true;
      return;
    }
   
    if(this->escapeNext) {
      this->d = this->d^0x20;
      this->escapeNext = false;
    }
    
    int i;   
    switch(this->readStatus) {
      case RSP_WAIT:
        if(this->d==0x7E) {
          this->readStatus = RSP_LMSB;
          this->checkSum = 0; // rESET CHECKSUM
        }
        break;
      case RSP_LMSB:
        this->lMSB = this->d;
        this->readStatus = RSP_LLSB;
        break;
      case RSP_LLSB:
        this->lLSB = this->d;
        
        this->packetSize = this->lMSB;
        this->packetSize = this->lMSB<<8;
        this->packetSize = this->packetSize | this->lLSB;
        
        if(this->packetSize>0) {
          // Free previous buffer
          free(this->rcvBuffer);
	  // Allocate new buffer
          this->rcvBuffer = (byte*)malloc(this->packetSize);
          this->rcvSize = 0;
        } else {
          // Critical error, size cannot be 0
          // reset machine status to RSP_WAIT
          this->readStatus = RSP_WAIT;
          break;
        }
        
        this->readStatus = RSP_RDDATA;
        break;
      case RSP_RDDATA:
        if( this->rcvSize<this->packetSize ) { // read data
          this->rcvBuffer[this->rcvSize++] = this->d;
          this->checkSum += this->d; // Calculate Checksum on the fly
        } else {
          // Last byte is the checksum
          this->checkSum += this->d;
          this->checkSum = 0xFF & this->checkSum;
          if(this->checkSum==0xFF) {
            if(this->frameReceivedHandler!=NULL) {
              this->frameReceivedHandler(this->rcvBuffer,this->rcvSize);
            }
            this->processFrame();
          }
          this->readStatus = RSP_WAIT; 
        }
        break;
    }
  } else if(this->readStatus==RSP_WAIT) {
    // Process queued messages
    if(this->queued16) {
      this->_sendTo16(this->queuedAddr, this->queuedString16);
      this->queued16 = false;
      this->queuedAddr = 0;
      this->queuedString16 = NULL;
    }
    if(this->queued64) {
      this->_sendTo64(this->queuedAddrHigh,this->queuedAddrLow, this->queuedString64);
      this->queued64 = false;
      this->queuedAddrHigh = 0;
      this->queuedAddrLow = 0;
      this->queuedString64 = NULL;
    }
  }
}

void XBeeSeries1::onFrameReceived(void (*handler)(uint8_t *dataFrame, uint16_t dataSize)) {
  this->frameReceivedHandler = handler;
}

void XBeeSeries1::onDataReceived16(void (*handler)(uint16_t addr, uint8_t *data, uint16_t dataSize)) {
  this->rx16Handler = handler;
}
void XBeeSeries1::onDataReceived64(void (*handler)(uint32_t addr_high,uint32_t addr_low, uint8_t *data, uint16_t dataSize)) {
  this->rx64Handler = handler;
}

void XBeeSeries1::onDataReceived(void (*handler)(uint8_t *data, uint16_t dataSize)) {
  this->rxHandler = handler;
}

void XBeeSeries1::onTXStatus(void (*handler)(uint8_t seq, uint8_t code)) {
  this->txStatHandler = handler;
}


bool XBeeSeries1::waitFor(char *response, void (*command)()) {
  this->lastSerialData = millis();
  
  char c;
  
  boolean responseFound = false;
  
  while((millis()-this->lastSerialData)<WAIT_TIMEOUT & !responseFound) {
    while(this->_HardSerial->available()) {
      c = this->_HardSerial->read();
      this->append_buffer(c);
      if(c=='\r') {
        if(strstr(this->buffer,response)!=0) {
          responseFound = responseFound | true;
          (*command)();
        }
        this->flush_buffer();
      }
    }
  }
  
  return responseFound;
}

bool XBeeSeries1::waitFor(char *response) {
  this->lastSerialData = millis();
  
  boolean responseFound = false;
  
  char c;
  while((millis()-this->lastSerialData)<WAIT_TIMEOUT & !responseFound) {
    while(this->_HardSerial->available()) {
      c = this->_HardSerial->read();
      this->append_buffer(c);
      if(c=='\r') {
        if(strstr(this->buffer,response)!=0) {
          responseFound = responseFound | true;
        }
        this->flush_buffer();
      }
    }
  }
  
  return responseFound;
}

void XBeeSeries1::flush_buffer() {
  for(int j=0;j<=this->buffPos;j++) {
    this->buffer[j] = 0;
  }
  this->buffPos = 0;
}

void XBeeSeries1::append_buffer(char c) {
  if(this->buffPos<BUFFSIZE) {
    this->buffer[this->buffPos++] = c;
  } else {
    flush_buffer();
  }
}

void XBeeSeries1::escapeAndWrite(uint8_t &data) {
  if(
    data == 0x7E |
    data == 0x7D |
    data == 0x11 |
    data == 0x13
    ) {
    this->_HardSerial->write(0x7D);
    this->_HardSerial->write(data^0x20);
  } else {
    this->_HardSerial->write(data);
  }
}

void XBeeSeries1::processFrame() {
  if(this->rcvSize==0) {
    return;
  }
  
  int i;
  uint16_t addr;
  uint32_t addr_high;
  uint32_t addr_low;
  switch(this->rcvBuffer[0]) {
    case 0x88:
      // TO DO: Process AT Command Response
      break;
    case 0x89:
        if(this->txStatHandler!=NULL) {
          this->txStatHandler(this->rcvBuffer[1], this->rcvBuffer[2]);
        }
      break;
    case 0x8A:
        // TO DO: Process status message
      break;
    case 0x80:
          addr_high = rcvBuffer[1]<<24;
          addr_high |= rcvBuffer[2]<<16;
          addr_high |= rcvBuffer[3]<<8;
          addr_high |= rcvBuffer[4];
          
          addr_low = rcvBuffer[5]<<24;
          addr_low |= rcvBuffer[6]<<16;
          addr_low |= rcvBuffer[7]<<8;
          addr_low |= rcvBuffer[8];
          
          if(this->rx64Handler!=NULL) {
            this->rx64Handler(addr_high, addr_low, (uint8_t*)(this->rcvBuffer+11), this->rcvSize-11);
          }
          if(this->rxHandler!=NULL) {
            this->rxHandler((uint8_t*)(this->rcvBuffer+11), this->rcvSize-11);
          }
      break;
    case 0x81:
          addr |= rcvBuffer[3]<<8;
          addr |= rcvBuffer[4];
          // Process data from 16bit address
          if(this->rx16Handler!=NULL) {
            this->rx16Handler(addr, (uint8_t*)(this->rcvBuffer+5), this->rcvSize-5);
          }
          if(this->rxHandler!=NULL) {
            this->rxHandler((uint8_t*)(this->rcvBuffer+5), this->rcvSize-5);
          }
      break;
    case 0x97:
        // TO DO: call process response handler
      break;
  }
}

