#pragma once
#include <Arduino.h>
#include <Preferences.h>

extern Preferences prefs;
extern const long SaveInterval;
extern const char *prefsPos;

void saveToPrefs(unsigned long &lastSave);
void initPrefs();
