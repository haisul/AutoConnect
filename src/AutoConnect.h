#ifndef __WIFICONTROL_H_
#define __WIFICONTROL_H_
#include "ArduinoJson.h"
#include "LITTLEFSFUN/littlefsfun.h"
// #include <ArduinoJson.h>
#include <WiFi.h>

class AutoConnect {
private:
    WiFiClass &pWiFi;

    static AutoConnect *instance;
    static void _wifiEvent(WiFiEvent_t);

public:
    AutoConnect(WiFiClass &);
    StaticJsonDocument<96> getWifiAuthen();
    bool saveWifiAuthen(String, String);
    bool removeWifiAuthen();
};
extern AutoConnect *instance;

#endif