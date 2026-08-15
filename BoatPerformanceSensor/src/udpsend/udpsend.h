#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

void initUDP();
void sendSensorDataUDP(float lift, float trim, float rpm, int overheat, int oilLow, float kn, float waterPressure, float fuel, float roll, float pitch, uint32_t satellites, double lat, double lng);