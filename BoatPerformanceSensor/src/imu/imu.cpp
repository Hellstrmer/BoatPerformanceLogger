#include "Arduino.h"
#include "imu.h"
#include <wire.h>

#define IMU_SDA 13
#define IMU_SCL 14
#define BNO_CS 12
#define BNO_INT 11
#define BNO_RESET 9

euler_t ypr;  
float accelX, accelY, accelZ;

Adafruit_BNO08x imu(BNO_RESET);
sh2_SensorValue_t sensorValue;

sh2_SensorId_t reportType = SH2_ARVR_STABILIZED_RV;
long reportIntervalUs = 20000;

bool imuReady = false;

void setReports()
{
    Serial.println("Setting desired reports");
    if (!imu.enableReport(SH2_ARVR_STABILIZED_RV, reportIntervalUs))
    {
        Serial.println("Could not enable rotation vector");
    }
    if (!imu.enableReport(SH2_LINEAR_ACCELERATION, reportIntervalUs))
    {
        Serial.println("Could not enable linear acceleration");
    }
}

void initIMU()
{
    Wire.begin(IMU_SDA, IMU_SCL);

    if (!imu.begin_I2C(0x4B, &Wire))
    {
        Serial.println("VARNING: BNO085 hittades inte, fortsätter utan IMU");
        imuReady = false;
        return;
    }
    imuReady = true;
    Serial.println("BNO085 Found!");
    setReports();
}

void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t *ypr, bool degrees = false)
{

    float sqr = sq(qr);
    float sqi = sq(qi);
    float sqj = sq(qj);
    float sqk = sq(qk);

    ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
    ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
    ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

    // radianer -> grader
    ypr->yaw *= RAD_TO_DEG;
    ypr->pitch *= RAD_TO_DEG;
    ypr->roll *= RAD_TO_DEG;
}

void imuUpdate()
{
    if (!imuReady) {
        static unsigned long lastRetry = 0;
        if (millis() - lastRetry > 5000) {   // försök igen var 5:e sekund
            lastRetry = millis();
            initIMU();                         // tyst återanslutningsförsök
        }
        return;                                // oavsett: gör inget mer denna gång
    }

    if (imu.wasReset())
    {
        setReports(); // Aktivera rapport om chipet återställts
    }
    if (imu.getSensorEvent(&sensorValue))
    {
        switch (sensorValue.sensorId)
        {
        case SH2_ARVR_STABILIZED_RV:
            quaternionToEuler(sensorValue.un.arvrStabilizedRV.real,
                              sensorValue.un.arvrStabilizedRV.i,
                              sensorValue.un.arvrStabilizedRV.j,
                              sensorValue.un.arvrStabilizedRV.k,
                              &ypr);
            break;
        case SH2_LINEAR_ACCELERATION:
            accelX = sensorValue.un.linearAcceleration.x;
            accelY = sensorValue.un.linearAcceleration.y;
            accelZ = sensorValue.un.linearAcceleration.z;
            break;
        }
    }
}
