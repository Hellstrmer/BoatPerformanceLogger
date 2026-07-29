#pragma once
#include <Arduino.h>

const byte pinA = 32;
const byte pinB = 12;
const float mmPerPulse = 0.005;
extern volatile long position;

void initEncoder();
float encoderGetPositionMM();
void encoderReset();