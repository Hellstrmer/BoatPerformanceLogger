#include <Arduino.h>

#include "encoder/encoder.h"
#include "espnow/espnow.h"
#include "prefs/prefs.h"
#include "rpm/rpm.h"
#include "alarm/alarm.h"

unsigned long sendInterval = 50; //20HZ
unsigned long meassureInterval = 100; // 10HZ


void setup()
{
  Serial.begin(115200);
  initPrefs();
  initEncoder();
  initESPNow();
  initRPM();
  initAlarms();
  delay(100);
}

void loop()
{  
  static unsigned long lastSendInterval = 0;
  static unsigned long lastReadInterval = 0;
  static unsigned long lastSave = 0;

  
 if (millis() - lastReadInterval > meassureInterval) {
    RPM = readRPM();
    readAlarms();
    lastReadInterval = millis();
}

 if (millis() - lastSendInterval > sendInterval) {
    float posMM = encoderGetPositionMM();
    sendSensorData(posMM, RPM, overheat, oilLow);    
    lastSendInterval = millis();
}
  // Save all values to internal memory
  // Enables memory for powerloss
  saveToPrefs(lastSave);
}
