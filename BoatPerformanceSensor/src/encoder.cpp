#include "encoder.h"

volatile long position = 0;


static void encoderA()
{
  // Count pulses of magnetic strip when moving up
  if (digitalRead(pinA) == digitalRead(pinB))  
    position++;  
  else  
    position--;  
}

static void encoderB()
{
  // Count pulses of magnetic strip when moving down
  if (digitalRead(pinA) != digitalRead(pinB))  
    position++;  
  else  
    position--;  
}


void initEncoder()
{
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinA), encoderA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinB), encoderB, CHANGE);  
}

float encoderGetPositionMM()
{
  noInterrupts();
  long pos = position;
  interrupts();
  return pos * mmPerPulse;
}

void encoderReset()
{
  noInterrupts();
  position = 0;
  interrupts();
}