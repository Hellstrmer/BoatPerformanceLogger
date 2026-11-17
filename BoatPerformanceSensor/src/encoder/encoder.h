#pragma once
#include <Arduino.h>

const byte pinA = 15;
const byte pinB = 16;
const float mmPerPulse = 0.005;
extern volatile long position;

void initEncoder();
float encoderGetPositionMM();
void encoderReset();