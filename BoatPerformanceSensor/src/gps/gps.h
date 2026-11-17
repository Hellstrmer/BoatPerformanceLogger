#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>


extern double gpsLat, gpsLng;
extern float gpsKn;
extern uint32_t gpsSatellites;
extern bool gpsValid;


void initGPS();
//void checkLocation();
void gpsUpdate();
uint32_t getGPSsatellites();