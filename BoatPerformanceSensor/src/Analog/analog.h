#pragma once
#include <Arduino.h>

void analogInit();
void analogUpdate();
void checkAnalogValues();

extern float g_battV;
extern float g_trim, g_trimRatio; // 0-100% trim, 0-1 trimkvot
extern float g_tank, g_tankRatio; // 0-100% tank, 0-1 tankkvot
extern float g_waterPressureV, g_waterPressureBar; // 0-1V water pressure, 0-30bar water pressure ratio
//extern float g_waterTemp, g_waterTempRatio; // 0-100% water temp, 0-1 water temp ratio

