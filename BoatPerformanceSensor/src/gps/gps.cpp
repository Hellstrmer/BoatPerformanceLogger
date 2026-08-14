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

void gpsUpdate()
{
    while (gpsSerial.available())
    {
        gps.encode(gpsSerial.read());
    }
}

float gpsSpeed()
{
    if (gps.speed.isValid())
    {
        float knots = gps.speed.knots();
        return knots;
    }
    return 0.0;
}

double gpsLocation()
{
    if (gps.location.isValid())
    {
        double lat = gps.location.lat();
        double lng = gps.location.lng();
        return lat, lng;
    }
    return 0.0, 0.0;
}