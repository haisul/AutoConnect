#pragma once

#ifndef __AUTOCONNECT_H_
#define __AUTOCONNECT_H_

#include "ArduinoJson.h"
#include "LITTLEFSFUN/littlefsfun.h"
#include "loggerESP/loggerESP.h"
#include <WiFi.h>

class AutoConnect {
private:
    WiFiClass &pWiFi;
    WiFiServer server;

    static AutoConnect *instance;
    static void _wifiEvent(WiFiEvent_t);

    StaticJsonDocument<96> _getWifiAuthen();

    void (*wifiConnectedCallback)(void);

    ///////////////////////////////////////////////////

    void _beginServer();

    TaskHandle_t taskHandle;
    static void taskFunctionStart(void *pvParam);
    void serverLoop();

    bool _getWifiAuthenFromServer(String);
    bool _tryConnect(String, String);

public:
    AutoConnect(WiFiClass &);

    void begin();
    bool saveWifiAuthen(String, String);
    bool removeWifiAuthen();
    void setWifiConnectedCallback(void (*callback)(void));
};
extern AutoConnect *instance;

#endif