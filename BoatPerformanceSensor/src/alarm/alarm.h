#pragma once
#include <Arduino.h>

#define PIN_OVERHEAT 24 //Update this
#define PIN_OIL 10 //Update this

extern bool overheat;
extern bool oilLow;

void initAlarms();
void readAlarms();
