#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>


extern double gpsLat, gpsLng;
extern bool gpsValid;

void initGPS();
void checkLocation();
void gpsUpdate();
uint32_t getGPSsatellites();