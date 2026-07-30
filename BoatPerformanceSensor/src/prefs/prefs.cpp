#include "Arduino.h"
#include "prefs.h"
#include "encoder/encoder.h"

Preferences prefs;
const char *prefsPos = "pos";
const long SaveInterval = 5000;

void saveToPrefs(unsigned long &lastSave)
{
  if (millis() - lastSave > SaveInterval)
  {
    noInterrupts();
    long pos = position;
    interrupts();
    prefs.putLong(prefsPos, pos);
    lastSave = millis();
  }
}
  // Start the prefs memory and ready out all values
void initPrefs()
{
    prefs.begin("hydrolift", false);
    position = prefs.getLong(prefsPos, 0);
}