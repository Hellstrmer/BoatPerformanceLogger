#pragma once
#include <Arduino.h>


// Magnetic enocder variables
const byte pinA = 32;
const byte pinB = 12;
const float mmPerPulse = 0.005;
volatile long position = 0;