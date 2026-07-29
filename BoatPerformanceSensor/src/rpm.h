#pragma once
#include <Arduino.h>

#define RPM_PIN 34 //Update this
#define PULSES_PER_REV 6.0f //Update this // 12 poler / 2 — Kalibreras mot originalvarvräknare
#define PCNT_UNIT PCNT_UNIT_0

extern float RPM;

void initRPM();
float readRPM();