#include <Arduino.h>

#include "encoder/encoder.h"
//#include "espnow/espnow.h"
#include "prefs/prefs.h"
//#include "rpm/rpm.h"
#include "alarm/alarm.h"
#include "udpsend/udpsend.h"

unsigned long sendInterval = 50; //20HZ
unsigned long meassureInterval = 100; // 10HZ

// DEMO
float RPMDEMO = 2100.0;
float kn = 0.0;


void setup()
{
  Serial.begin(115200);
  initPrefs();
  //initEncoder();
  //initESPNow();
  delay(500);
  initUDP();
  //initRPM();
  initAlarms();
  delay(100);
}

void loop()
{  
  static unsigned long lastSendInterval = 0;
  static unsigned long lastReadInterval = 0;
  static unsigned long lastSave = 0;

  
 if (millis() - lastReadInterval > meassureInterval) {
    //RPM = readRPM();
    readAlarms();
    lastReadInterval = millis();
}

 if (millis() - lastSendInterval > sendInterval) {
    float lift = encoderGetPositionMM();
    // Demo Data
    lift = 120;
    float trim = 10.0;
    float waterPreassure = 10.0;
    float fuel = 25.0;
    // if (RPMDEMO > 6200.0)
    // {
    //   RPMDEMO = 2000.0;
    // }
    // RPMDEMO += 30.0;
    RPMDEMO = 6000.0;
    kn = (RPMDEMO / 100 - 2);
    sendSensorDataUDP(lift, trim, RPMDEMO, overheat, oilLow, kn, waterPreassure, fuel);    

    lastSendInterval = millis();
}
  // Save all values to internal memory
  // Enables memory for powerloss
  saveToPrefs(lastSave);
}
