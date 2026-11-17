#include "udpsend.h"

static const char* PI_SSID = "HydroliftPi";
static const char* PI_PASS = "hydrolift";
static IPAddress PiIP(10,42,0,1);
static const uint16_t PI_PORT = 5005;

WiFiUDP udp;

JsonDocument doc;

void initUDP()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(PI_SSID, PI_PASS);
    delay(1000);

    Serial.print("Ansluter till ");
    Serial.println(PI_SSID);    
    int tries = 0;
    
    while (WiFi.status() != WL_CONNECTED || WiFi.localIP()[0] == 0)
    {
        delay(500);
        Serial.print("*");
        tries++;
        if (tries % 20 == 0) 
        {
            Serial.println("\nFörsöker igen...");
            WiFi.disconnect();
            WiFi.begin(PI_SSID, PI_PASS);
        }
    }
    Serial.println("Wifi Connected!");
 
}

static void CheckConnection() 
{
    static unsigned long lastTry = 0;
    if (WiFi.status() != WL_CONNECTED && millis() - lastTry > 2000)
        {
            WiFi.reconnect();
            lastTry = millis();
        }        
}

// Skickar null i stället för NaN, så visar dashen ett streck
static void putNum(const char* key, float v)
{
    if (isnan(v)) doc[key] = nullptr;
    else          doc[key] = v;
}

void sendSensorDataUDP(float lift, float trim, float rpm, int overheat, int oilLow, float kn, float waterPressure, float fuel, float roll, float pitch, uint32_t satellites, double lat, double lng, float batteryVoltage, float tankLevel)
{
    CheckConnection();
    // Convert to JSON
    doc["ms"] = millis();
    doc["lift"] = lift;
    putNum("trim", trim);
    doc["rpm"] = rpm;
    doc["overheat"] = overheat;
    doc["oilLow"] = oilLow;
    doc["kn"] = kn;
    doc["waterpressure"] = waterPressure;
    doc["fuel"] = fuel;
    doc["roll"] = roll;
    doc["pitch"] = pitch;
    doc["satellites"] = satellites;
    doc["lat"] = lat;
    doc["lng"] = lng;
    putNum("battery", batteryVoltage);
    putNum("tank_level", tankLevel);
    

    char buf[512];
    size_t n = serializeJson(doc, buf);


    udp.beginPacket(PiIP, PI_PORT);
    udp.write((uint8_t*)buf, n);
    udp.endPacket();

}



