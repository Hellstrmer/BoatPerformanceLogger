#include <Arduino.h>

#include "encoder.h"
#include "espnow.h"
#include "prefs.h"
#include "rpm.h"

unsigned long sendInterval = 50; //20HZ
unsigned long meassureInterval = 100; // 10HZ


void setup()
{
  Serial.begin(115200);
  initPrefs();
  initEncoder();
  initESPNow();
  initRPM();
  delay(100);
}

void loop()
{  
  static unsigned long lastSendInterval = 0;
  static unsigned long lastReadInterval = 0;
  static unsigned long lastSave = 0;

  
 if (millis() - lastReadInterval > meassureInterval) {
    RPM = readRPM();
    lastReadInterval = millis();
}

 if (millis() - lastSendInterval > sendInterval) {
    float posMM = encoderGetPositionMM();
    sendSensorData(posMM, RPM);
    lastSendInterval = millis();

}
  // Save all values to internal memory
  // Enables memory for powerloss
  saveToPrefs(lastSave);
}
