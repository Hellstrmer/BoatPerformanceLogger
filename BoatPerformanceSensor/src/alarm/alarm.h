#pragma once
#include <Arduino.h>

#define PIN_OVERHEAT 32 //Update this
#define PIN_OIL 31 //Update this

extern bool overheat;
extern bool oilLow;

void initAlarms();
void readAlarms();
