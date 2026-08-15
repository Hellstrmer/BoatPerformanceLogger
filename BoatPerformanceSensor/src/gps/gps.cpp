#include "Arduino.h"
#include "gps.h"

TinyGPSPlus gps;

HardwareSerial gpsSerial(1);

void initGPS()
{
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
}

void checkLocation()
{
    if (gps.location.isValid())
    {
        Serial.printf("Sats:%d Fart:%.1f kn Pos:%.5f,%.5f\n",
                      gps.satellites.value(), gps.speed.knots(),
                      gps.location.lat(), gps.location.lng());
    }
    else
    {
        Serial.printf("Söker satelliter... sats:%d\n", gps.satellites.value());
    }
    delay(1000);
}

// void gpsUpdate()
// {
//     while (gpsSerial.available())
//     {
//         gps.encode(gpsSerial.read());
//     }
// }

float gpsSpeed()
{
    if (gps.speed.isValid())
    {
        float knots = gps.speed.knots();
        return knots;
    }
    return 0.0;
}

double gpsLat = 0.0, gpsLng = 0.0;
bool gpsValid = false;

void gpsUpdate() {
    while (gpsSerial.available()) gps.encode(gpsSerial.read());
    if (gps.location.isValid()) {
        gpsLat = gps.location.lat();
        gpsLng = gps.location.lng();
        gpsValid = true;
    }
}
uint32_t getGPSsatellites() {
    return gps.satellites.value();
}