#include <Arduino.h>

bool checkTimeout(long &var,long target) {
  long currTime = millis();
  boolean timedOut = ((currTime-var)>target) || (currTime-var<0);
  
  if(timedOut) {
    var = currTime;
  }
  
  return timedOut;
}
