# AutoConnect

An Arduino program for ESP32/ESP8266 to connect to WiFi using a mobile app.

Prerequisites:

1.ArduinoIDE

2.ESP32/ESP8266

3.mobile app Link:


Example:


```
#include "AutoConnect.h"
#include <Arduino.h>
#include <WiFi.h>

AutoConnect *p_autoConnect;

IPAddress localIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

void onWifiConnected() {
    Logger(LI, "WIFI is connected!");
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESP_32");
    WiFi.softAPConfig(localIP, gateway, subnet);

    p_autoConnect = new AutoConnect(WiFi);
    p_autoConnect->setWifiConnectedCallback(onWifiConnected);
    p_autoConnect->begin();
}

String msgBuffer = "";

void loop() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n') {
            msgBuffer.trim();
            if (msgBuffer == "reset") {
                p_autoConnect->removeWifiAuthen();
                ESP.restart();
            }
            msgBuffer = "";
            break;
        } else
            msgBuffer += c;
    }
}
```
