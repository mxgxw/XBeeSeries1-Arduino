About
===================

This is a really simple and small library to control the
XBee Series 1 wireless module.

This library attach itself to an HardwareSerial port.

If you are using SoftSerial ports or similars this library is
not going to work.


Installation & Examples
===================

1. Copy TeUbico_XBeeS1 to your Arduino *"libraries"* folder. 
2. Restart Arduino.
3. Check out the examples under *Examples* > *TeUbico_XBee*.
4. Profit!

Basic Setup
===================

To use the library you need to follow this steps:

1. Include the following headers:
```
#include <teubico_XBeeS1.h>
#include <teubico_utils.h>
```
2. Create a global pointer of type XBeeS1:
```
XBeeS1 * myXBee;
```
3. Under setup() create a new instance of XBeeS1 passing the Serial
   port pointer as the parameter of the constructor and call init().
```
void setup() {
  myXBee = new XBeeS1(&Serial);
  myXBee->init();
}
```
4. Call listen() under loop().
```
void loop() {
  myXBee->listen();
}
```

Sending data
===================

The library provides three functions to send data:

```
sendTo16(uint16_t addr, char *data);
```
Sends data to the specified 16 bit address.

```
sendTo64(uint32_t addr_high,uint32_t addr_low, char *data);
```
Sends data to the specified 64 bit address.


```
broadcast(uint32_t addr_low, char *data);
```
Sends a broadcast to the local PAN. Internally is a shorcut
to sendTo16(0xFFFF,char *data)

*Important note*

Try to not block the main loop, this could cause troubles.
Minimize the use of wait and long loops.

For example this code is preferred:
```
long lastMessage = 0;
void loop() {
  myXBee->listen();
  

  if(checkTimeout(lastMessage,1000)) {
    myXBee->sendTo64(0x00,0xFFFF,"Hello World\r\n");
    lastMessage = millis();
  }
}
```
Over:
```
void loop() {
  myXBee->listen();
  
  myXBee->sendTo64(0x00,0xFFFF,"Hello World\r\n");
  delay(1000);
}
```
Again, *DO NOT BLOCK* the loop() thread. Try to program
to simulate a "cooperative multitasking", use counters to
keep track of the time and try to not use very long loops
because this could affect the performance or in the worst
case the library is not going to work.

Receiving data
===================

This is a Event-Driven library, to receive data you must
speficy a handler to process the data before initialization.

Note: You can specify it at any time but it's better to define
the handlers at the initialization.

You have available three data reception handlers:

```
onDataReceived16(void (*handler)(uint16_t addr, uint8_t *data, uint16_t dataSize));
onDataReceived64(void (*handler)(uint32_t addr_high,uint32_t addr_low, uint8_t *data, uint16_t dataSize));
onDataReceived(void (*handler)(uint8_t *data, uint16_t dataSize));

```
The first one is called when data is received on the 16 bit address
of the XBee module.

The second one is called when data is received on the 64 bit address of the
XBee module.

The last one is called after data is received independent of the address used.

Example:

```
// On setup:
myXBee->onDataReceived16(dataReceived16);
myXBee->onDataReceived64(dataReceived64);
myXBee->onDataReceived64(dataReceived);

// Expects:
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

void dataReceived(uint8_t *data, uint16_t dataSize) {
  for(int i=0; i<dataSize; i++) {
    Serial.write(data[i]);
  }
}
```

Other Info
===================

There are handlers for TX status and if you want to implement a custom frame processor
try to use the onFrameReceived handler. This is called just before trying to identify
the API command in the frame.


License
===================

    TeUbi.co XBee a simple way to use XBee in API Mode.
    Copyright (C) 2014  Mario Gomez < mario.gomez _at- teubi.co >
    
    TeUbi.co XBee is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    TeUbi.co XBee is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
