#include <Arduino.h>
#include "alarm.h"

bool overheat = false;
bool oilLow = false;

void initAlarms() {
  pinMode(PIN_OVERHEAT, INPUT_PULLUP);
  pinMode(PIN_OIL, INPUT_PULLUP);
}

void readAlarms() {
  overheat = (digitalRead(PIN_OVERHEAT) == LOW);
  oilLow   = (digitalRead(PIN_OIL) == LOW);
}