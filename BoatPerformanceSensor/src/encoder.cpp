#include "encoder.h"


void encoderInit()
{
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinA), encoderA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinB), encoderB, CHANGE);  
}

void encoderA()
{
  // Count pulses of magnetic strip when moving up
  if (digitalRead(pinA) == digitalRead(pinB))  
    position++;  
  else  
    position--;  
}

void encoderB()
{
  // Count pulses of magnetic strip when moving down
  if (digitalRead(pinA) != digitalRead(pinB))  
    position++;  
  else  
    position--;  
}
