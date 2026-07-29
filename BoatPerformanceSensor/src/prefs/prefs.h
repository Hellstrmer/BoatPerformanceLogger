#pragma once
#include <Arduino.h>
#include <Preferences.h>

Preferences prefs;
const long SaveInterval = 5000;
const char *prefsPos = "pos";

void saveToPrefs(unsigned long &lastSave);
void initPrefs();
