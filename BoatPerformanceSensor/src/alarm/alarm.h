#pragma once
#include <Arduino.h>

#define PIN_OVERHEAT 38 //Update this
#define PIN_OIL 21 //Update this

extern bool overheat;
extern bool oilLow;

void initAlarms();
void readAlarms();
