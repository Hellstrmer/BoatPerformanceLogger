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
// *********************** DEMO ********************************
void sendDemoData() {
    // Faser: ramp upp → håll fullt → ramp ner → håll tomgång → repeat
    static unsigned long phaseStart = 0;
    static int phase = 0;           // 0=upp, 1=full, 2=ner, 3=tomgång
    static float t = 0.0;           // 0..1, position i turen

    const unsigned long RAMP_UP   = 8000;   // ms att gasa upp
    const unsigned long HOLD_FULL = 5000;   // ms på full gas
    const unsigned long RAMP_DOWN = 6000;   // ms att varva ner
    const unsigned long HOLD_IDLE = 3000;   // ms på tomgång

    unsigned long now = millis();
    unsigned long elapsed = now - phaseStart;

    switch (phase) {
        case 0:  // ramp upp
            t = (float)elapsed / RAMP_UP;
            if (t >= 1.0) { t = 1.0; phase = 1; phaseStart = now; }
            break;
        case 1:  // håll fullt
            t = 1.0;
            if (elapsed > HOLD_FULL) { phase = 2; phaseStart = now; }
            break;
        case 2:  // ramp ner
            t = 1.0 - (float)elapsed / RAMP_DOWN;
            if (t <= 0.0) { t = 0.0; phase = 3; phaseStart = now; }
            break;
        case 3:  // håll tomgång
            t = 0.0;
            if (elapsed > HOLD_IDLE) { phase = 0; phaseStart = now; }
            break;
    }

    // ---- härled alla värden från t (0=tomgång, 1=full fart) ----
    float RPMDEMO = 2000.0 + t * 4130.0 + random(-30, 30);

    float kn = (RPMDEMO / 100.0 - 2.0) * 0.92 + random(-5, 5) / 10.0;
    if (kn < 0) kn = 0;

    float lift = 154.0 - t * 9.0 + random(-3, 3) / 10.0;
    float trim = t * 10.2 + random(-2, 2) / 10.0;
    float waterPreassure = 10.0 + random(-5, 5) / 10.0;
    float fuel = 8.0 + t * 44.0;

    sendSensorDataUDP(lift, trim, RPMDEMO, overheat, oilLow, kn, waterPreassure, fuel);
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
    // float lift = encoderGetPositionMM();
    // // Demo Data
    // lift = 120;
    // float trim = 10.0;
    // float waterPreassure = 10.0;
    // float fuel = 25.0;
    // // if (RPMDEMO > 6200.0)
    // // {
    // //   RPMDEMO = 2000.0;
    // // }
    // // RPMDEMO += 30.0;
    // RPMDEMO = 6000.0;
    // kn = (RPMDEMO / 100 - 2);
    // sendSensorDataUDP(lift, trim, RPMDEMO, overheat, oilLow, kn, waterPreassure, fuel);    
    sendDemoData();

    lastSendInterval = millis();
}
  // Save all values to internal memory
  // Enables memory for powerloss
  saveToPrefs(lastSave);
}
