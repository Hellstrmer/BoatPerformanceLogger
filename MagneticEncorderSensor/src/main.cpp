#include <Arduino.h>
#include <SPI.h>
#include <Preferences.h>
#include <esp_now.h>
#include <WiFi.h>
#include "driver/pcnt.h"

uint8_t receiverMAC[] = {0x54, 0x32, 0x04, 0x33, 0x62, 0xC4};
struct SensorData {
  float posMM;
};

// Magnetic enocder variables
const byte pinA = 32;
const byte pinB = 12;
const float mmPerPulse = 0.005;
volatile long position = 0;

// RPM Variables
#define RPM_PIN 34 //Update this
#define PULSES_PER_REV 6.0f //Update this // 12 poler / 2 — Kalibreras mot originalvarvräknare
#define PCNT_UNIT PCNT_UNIT_0


// Prefs memory variables
Preferences prefs;
const long SaveInterval = 5000;
const char *prefsPos = "pos";

void encoderA()
{
  // Count pulses of magnetic strip when moving up
  if (digitalRead(pinA) == digitalRead(pinB))  
    position++;  
  else  
    position--;  
}

void encoderB()
{
  // Count pulses of magnetic strip when moving down
  if (digitalRead(pinA) != digitalRead(pinB))  
    position++;  
  else  
    position--;  
}

// RPM
void initRPM() {
  pcnt_config_t cfg = {};
  cfg.pulse_gpio_num = RPM_PIN;
  cfg.ctrl_gpio_num = PCNT_PIN_NOT_USED;
  cfg.channel = PCNT_CHANNEL_0;
  cfg.unit = PCNT_UNIT;
  cfg.pos_mode = PCNT_COUNT_INC;
  cfg.neg_mode = PCNT_COUNT_DIS;
  cfg.counter_h_lim = 32767;
  cfg.counter_l_lim = 0;
  pcnt_unit_config(&cfg);

  
}

struct configData {
  float lowerColorMax;
  float upperColorMin;
  bool resetPosition;
};

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

void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  configData config;
  memcpy(&config, data, sizeof(config));

  if (config.resetPosition) {
    position = 0;
  }
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESPNow init failed!");
    return;
  }
  esp_now_register_recv_cb(onDataReceived);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void sendSensorData(float posMM) {
  SensorData data = {posMM};
  esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));
}

void setup()
{
  Serial.begin(115200);
  // Start the prefs memory and ready out all values
  prefs.begin("hydrolift", false);
  position = prefs.getLong(prefsPos, 0);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinA), encoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), encoderB, CHANGE);
  initESPNow();
  delay(100);
}

void loop()
{  
  static unsigned long lastSend = 0;
  static unsigned long lastSave = 0;
 if (millis() - lastSend > 50) {
    noInterrupts();
    long pos = position;
    interrupts();
    float posMM = pos * mmPerPulse;
    sendSensorData(posMM);
    lastSend = millis();
}
  // Save all values to internal memory
  // Enables memory for powerloss
  saveToPrefs(lastSave);
}
