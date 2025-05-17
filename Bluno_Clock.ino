#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include "esp_wifi.h"

const int rPin = 3;
const int gPin = 4;
const int bPin = 5;

const char* WIFI_SSID = "Czekoladowa Rozkosz";
const char* WIFI_PASSWORD = "MlecznaDolina";
const unsigned int UDP_PORT = 4210;
const char* DEVICE_ID = "b4e91b7f";

WiFiUDP udp;

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
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("Connected!");
  Serial.printf("ESP32 IP: %s\n", WiFi.localIP().toString().c_str());

  esp_wifi_set_ps(WIFI_PS_NONE);

  if (MDNS.begin(DEVICE_ID)) {
    MDNS.addService("rgbapp", "udp", UDP_PORT);
    MDNS.addServiceTxt("rgbapp", "udp", "name", "Bluno Clock");
    MDNS.addServiceTxt("rgbapp", "udp", "id", DEVICE_ID);
    Serial.printf("mDNS started: %s.local\n", DEVICE_ID);
  } else {
    Serial.println("mDNS setup failed");
  }

  udp.begin(UDP_PORT);
  Serial.printf("UDP server listening on port %u\n", UDP_PORT);

  lastActivityTime = millis();
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    IPAddress srcIP = udp.remoteIP();
    unsigned int srcPort = udp.remotePort();

    char buffer[64];
    int len = udp.read(buffer, sizeof(buffer) - 1);
    if (len <= 0) return;
    buffer[len] = '\0';

    Serial.printf("📥 Received: %s from %s:%u\n", buffer, srcIP.toString().c_str(), srcPort);

    processRGBData(String(buffer));
    lastActivityTime = millis();

    udp.beginPacket(srcIP, srcPort);
    udp.write((uint8_t *)"\x01", 1);
    udp.endPacket();

    Serial.println("✅ Applied RGB and sent DONE (0x01)");
  }

  if (millis() - lastActivityTime >= 10000) {
    slowPulse();
    delay(25);
  }
}

void processRGBData(String data) {
  data.trim();
  if (data.startsWith("[") && data.endsWith("]")) {
    data = data.substring(1, data.length() - 1);
    int firstComma = data.indexOf(',');
    int secondComma = data.lastIndexOf(',');

    int r = data.substring(0, firstComma).toInt();
    int g = data.substring(firstComma + 1, secondComma).toInt();
    int b = data.substring(secondComma + 1).toInt();

    if (r >= 0 && g >= 0 && b >= 0) {
      analogWrite(rPin, 255 - r);
      analogWrite(gPin, 255 - g);
      analogWrite(bPin, 255 - b);
    }
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
