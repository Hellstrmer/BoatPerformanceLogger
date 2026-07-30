#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

void initUDP();
void sendSensorDataUDP(float lift, float trim, float rpm, bool overheat, bool oilLow, float kn, float waterPressure, float fuel);