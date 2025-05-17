#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include "esp_wifi.h"

const char* WIFI_SSID     = "";
const char* WIFI_PASSWORD = "";
const unsigned int UDP_PORT = 4210;
const char* DEVICE_ID = "b4e91b7f";

WiFiUDP udp;

int rPin = 3;
int gPin = 4;
int bPin = 5;

unsigned long lastActivityTime = 0;
int pulseValue = 0;
int pulseDirection = 1;

void setup() {
  Serial.begin(115200);
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  esp_wifi_set_ps(WIFI_PS_NONE);

  if (MDNS.begin(DEVICE_ID)) {
    MDNS.addService("rgbapp", "udp", UDP_PORT);
    MDNS.addServiceTxt("rgbapp", "udp", "name", "Bluno Clock");
    MDNS.addServiceTxt("rgbapp", "udp", "id", DEVICE_ID);
  }

  udp.begin(UDP_PORT);
  lastActivityTime = millis();
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize >= 3) {
    uint8_t buffer[3];
    int len = udp.read(buffer, 3);
    if (len == 3) {
      int r = buffer[0];
      int g = buffer[1];
      int b = buffer[2];

      analogWrite(rPin, 255 - r);
      analogWrite(gPin, 255 - g);
      analogWrite(bPin, 255 - b);

      lastActivityTime = millis();

      uint8_t doneByte[1] = { 0x01 };
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write(doneByte, 1);
      udp.endPacket();
    }
  }

  if (millis() - lastActivityTime >= 10000) {
    slowPulse();
    delay(25);
  }
}

void slowPulse() {
  pulseValue += pulseDirection * 5;
  if (pulseValue <= 0 || pulseValue >= 255) {
    pulseDirection = -pulseDirection;
  }

  analogWrite(rPin, pulseValue);
  analogWrite(gPin, pulseValue);
  analogWrite(bPin, pulseValue);
}
