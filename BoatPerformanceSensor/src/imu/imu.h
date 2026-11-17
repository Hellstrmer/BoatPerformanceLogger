#pragma once
#include <Arduino.h>
#include <Adafruit_BNO08x.h>

struct euler_t {
    float yaw;
    float pitch;
    float roll;
};

extern euler_t ypr;   
extern float accelX, accelY, accelZ;

void setReports();
void initIMU();
void imuUpdate();