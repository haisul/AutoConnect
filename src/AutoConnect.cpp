#include "AutoConnect.h"

AutoConnect *AutoConnect::instance = nullptr;
AutoConnect::AutoConnect(WiFiClass &WiFi) : pWiFi(WiFi) {
    instance = this;
    pWiFi.onEvent(_wifiEvent);
    StaticJsonDocument<96> j_wifi = getWifiAuthen();
    if (!j_wifi.isNull()) {
        String _ssid = j_wifi["SSID"].as<String>();
        String _pass = j_wifi["PASSWORD"].as<String>();
        pWiFi.begin(_ssid.c_str(), _pass.c_str());
    } else {
        pWiFi.beginSmartConfig();
        Serial.println("beginSmartConfig");
    }
}

StaticJsonDocument<96> AutoConnect::getWifiAuthen() {
    if (!initLittleFS()) {
        Serial.println("load wifi data Failed!");
        return StaticJsonDocument<96>();
    } else {
        StaticJsonDocument<96> j_wifi;
        String readWifiStr = readFile(LittleFS, "/wifi/wifi.txt");
        DeserializationError error = deserializeJson(j_wifi, readWifiStr);
        if (error) {
            Serial.printf("deserialize wifi data Failed: %s\n", error.c_str());
            return StaticJsonDocument<96>();
        }

        Serial.println(readWifiStr.c_str());
        Serial.println("WIFI data load complete");
        return j_wifi;
    }
}

bool AutoConnect::saveWifiAuthen(String ssid, String pass) {
    StaticJsonDocument<96> j_wifi;
    j_wifi["SSID"] = ssid;
    j_wifi["PASSWORD"] = pass;
    String wifiStr;
    serializeJson(j_wifi, wifiStr);
    writeFile2(LittleFS, "/wifi/wifi.txt", wifiStr.c_str());
    Serial.printf("Wifi data is saved\n%s\n", wifiStr.c_str());
    return true;
}

bool AutoConnect::removeWifiAuthen() {
    if (!initLittleFS()) {
        return false;
    } else {
        deleteFile(LittleFS, "/wifi/wifi.txt");
        Serial.println("Wifi data is removed");
        return true;
    }
}

void AutoConnect::_wifiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.print("\nWifi is connected!\n");
        Serial.println(instance->pWiFi.SSID());
        Serial.println(instance->pWiFi.psk());
        Serial.println(instance->pWiFi.localIP());
        Serial.println(instance->pWiFi.RSSI());
        instance->saveWifiAuthen(instance->pWiFi.SSID().c_str(), instance->pWiFi.psk().c_str());
        break;
    }
}