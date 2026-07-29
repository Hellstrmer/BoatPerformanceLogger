#include "espnow.h"
#include "encoder/encoder.h"
#include "rpm/rpm.h"
#include <WiFi.h>

uint8_t receiverMAC[] = {0x54, 0x32, 0x04, 0x33, 0x62, 0xC4};

void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  configData config;
  memcpy(&config, data, sizeof(config));

  if (config.resetPosition) 
  {
    encoderReset();
  }
}


void sendSensorData(float posMM, float RPM, bool overheat, bool oilLow) {
  SensorData data = {posMM, RPM, overheat, oilLow};
  esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));
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
