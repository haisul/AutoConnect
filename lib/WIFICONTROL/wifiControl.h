#ifndef __WIFICONTROL_H_
#define __WIFICONTROL_H_
#include "ArduinoJson.h"
#include "Littlefsfun/littlefsfun.h"
#include <WiFi.h>

class WifiControl {
private:
    WiFiClass &pWiFi;

    static WifiControl *instance;
    static void _wifiEvent(WiFiEvent_t);

public:
    WifiControl(WiFiClass &);
    StaticJsonDocument<96> getWifiAuthen();
    bool saveWifiAuthen(String, String);
    bool removeWifiAuthen();
};
extern WifiControl *instance;

#endif