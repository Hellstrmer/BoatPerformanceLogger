#include "Arduino.h"
#include "gps.h"

TinyGPSPlus gps;

HardwareSerial gpsSerial(1);

void initGPS()
{
    gpsSerial.begin(9600, SERIAL_8N1, 18, 17);
}

// void checkLocation()
// {
//     if (gps.location.isValid())
//     {
//         Serial.printf("Sats:%d Fart:%.1f kn Pos:%.5f,%.5f\n",
//                       gps.satellites.value(), gps.speed.knots(),
//                       gps.location.lat(), gps.location.lng());
//     }
//     else
//     {
//         Serial.printf("Söker satelliter... sats:%d\n", gps.satellites.value());
//     }
// }

double gpsLat = 0.0, gpsLng = 0.0;
float gpsKn = 0.0;
uint32_t gpsSatellites = 0;
bool gpsValid = false;

//Speed EMA-filter
float speedFiltered = 0.0;
const float SPEED_ALPHA = 0.2;

float filterSpeed(float raw) {
    speedFiltered = SPEED_ALPHA * raw + (1.0 - SPEED_ALPHA) * speedFiltered;
    return speedFiltered;
}

void gpsUpdate()
{
    while (gpsSerial.available())
        gps.encode(gpsSerial.read());
    if (gps.location.isValid())
    {
        gpsLat = gps.location.lat();
        gpsLng = gps.location.lng();
        gpsValid = true;
    }
    if (gps.speed.isValid())
    {
        gpsKn = filterSpeed(gps.speed.knots());
        //gpsKn = gps.speed.knots();
    }
    gpsSatellites = gps.satellites.value();
}
// uint32_t getGPSsatellites()
// {
//     return gps.satellites.value();
// }