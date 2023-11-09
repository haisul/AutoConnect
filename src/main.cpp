#include "wifiControl.h"
#include <Arduino.h>

WifiControl *p_wifiControl;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    p_wifiControl = new WifiControl(WiFi);
}

String msgBuffer = "";
void loop() {

    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n') {
            msgBuffer.trim();
            if (msgBuffer == "reset") {
                p_wifiControl->removeWifiAuthen();
            }
            msgBuffer = "";
            break;
        } else
            msgBuffer += c;
    }
}
