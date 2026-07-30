#include "udpsend.h"

static const char* PI_SSID = "HydroliftPi";
static const char* PI_PASS = "hydrolift";
static IPAddress PiIP(10,42,0,1);
static const uint16_t PI_PORT = 5005;

WiFiUDP udp;

JsonDocument doc;


char buf[256];
size_t n = serializeJson(doc, buf);

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

void sendSensorDataUDP(float posMM, float rpm, bool overheat, bool oilLow)
{
    CheckConnection();
    // Convert to JSON
    doc["ms"] = millis();
    doc["lift"] = posMM;
    doc["rpm"] = rpm;
    doc["overheat"] = overheat;
    doc["oilLow"] = oilLow;

    char buf[256];
    size_t n = serializeJson(doc, buf);


    udp.beginPacket(PiIP, PI_PORT);
    udp.write((uint8_t*)buf, n);
    udp.endPacket();

}

