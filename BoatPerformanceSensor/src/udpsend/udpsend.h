#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>



void initUDP();
void sendSensorDataUDP(float posMM, float rpm, bool overheat, bool oilLow);