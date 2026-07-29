#pragma once
#include <Arduino.h>
#include <esp_now.h>

extern uint8_t receiverMAC[6];

struct SensorData {
  float posMM;
  float RPM;
  bool overheat;
  bool oilLow;
};

struct configData {
  float lowerColorMax;
  float upperColorMin;
  bool resetPosition;
};

void initESPNow();
void sendSensorData(float posMM, float RPM, bool overheat, bool oilLow);
