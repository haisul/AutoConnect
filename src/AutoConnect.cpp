#include "AutoConnect.h"

AutoConnect *AutoConnect::instance = nullptr;
AutoConnect::AutoConnect(WiFiClass &WiFi) : pWiFi(WiFi) {
    instance = this;
    pWiFi.onEvent(_wifiEvent);
}

void AutoConnect::begin() {
    if (pWiFi.status() != WL_CONNECTED) {
        StaticJsonDocument<96> j_wifi = _getWifiAuthen();
        if (!j_wifi.isNull()) {
            String _ssid = j_wifi["SSID"].as<String>();
            String _pass = j_wifi["PASSWORD"].as<String>();
            pWiFi.begin(_ssid.c_str(), _pass.c_str());
            pWiFi.mode(WIFI_STA);
        } else {

            if (pWiFi.getMode() == WIFI_AP_STA) {
                _beginServer();
            }
        }
    } else {
        Logger(LW, "WIFI is already connected!");
    }
}

StaticJsonDocument<96> AutoConnect::_getWifiAuthen() {
    if (!initLittleFS()) {
        Logger(LE, "load wifi data Failed!");
        return StaticJsonDocument<96>();
    } else {
        StaticJsonDocument<96> j_wifi;
        String readWifiStr = readFile(LittleFS, "/wifi/wifi.txt");
        DeserializationError errorMsg = deserializeJson(j_wifi, readWifiStr);
        if (errorMsg) {
            Logger(LE, "deserialize wifi data Failed: %s", errorMsg.c_str());
            return StaticJsonDocument<96>();
        }

        Logger(LN, readWifiStr.c_str());
        Logger(LN, "WIFI data load complete");
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
    Logger(LN, "Wifi data is saved: %s", wifiStr.c_str());
    return true;
}

bool AutoConnect::removeWifiAuthen() {
    if (!initLittleFS()) {
        return false;
    } else {
        deleteFile(LittleFS, "/wifi/wifi.txt");
        Logger(LN, "Wifi data is removed");
        return true;
    }
}

void AutoConnect::_wifiEvent(WiFiEvent_t event) {
    Logger(LN, "[WiFi-event] event: %d", event);
    switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Logger(LN, "SSID:%s PSK:%s IP:%s RSSI:%d", instance->pWiFi.SSID(), instance->pWiFi.psk(), instance->pWiFi.localIP().toString().c_str(), instance->pWiFi.RSSI());
        instance->saveWifiAuthen(instance->pWiFi.SSID().c_str(), instance->pWiFi.psk().c_str());
        if (instance->wifiConnectedCallback) {
            instance->wifiConnectedCallback();
        }
        break;
    }
}

void AutoConnect::setWifiConnectedCallback(void (*callback)(void)) {
    wifiConnectedCallback = callback;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void AutoConnect::_beginServer() {
    Logger(LN, "WiFi Server Begin");
    AutoConnect *params = this;
    xTaskCreatePinnedToCore(taskFunctionStart, "serverLoop", 8192, (void *)params, 1, &taskHandle, 1);
}

void AutoConnect::taskFunctionStart(void *pvParam) {
    AutoConnect *instance = static_cast<AutoConnect *>(pvParam);
    instance->serverLoop();
    vTaskDelete(NULL);
}

void AutoConnect::serverLoop() {
    unsigned long previousMillis = 0;
    const long interval = 10000;

    server.begin(80);
    Logger(LN, "AP IP address: %s", WiFi.softAPIP().toString());

    while (1) {
        WiFiClient client = server.available();
        if (client) {
            while (client.connected()) {
                if (client.available()) {
                    String data = client.readStringUntil('\n');
                    if (data == "\r") {
                        String body = "";
                        while (client.available()) {
                            char c = client.read();
                            body += c;
                        }
                        Logger(LN, "Receive body: %s", body.c_str());

                        bool result = _getWifiAuthenFromServer(body);
                        Logger(LN, "Client disconnected");
                        if (result) {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Connection: close");
                            client.println("Content-Type: text/plain");
                            client.println();
                            client.println("Received body content: true" + body);
                            client.stop();
                            delay(2000);
                            pWiFi.mode(WIFI_STA);
                            return;
                        } else {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Connection: close");
                            client.println("Content-Type: text/plain");
                            client.println();
                            client.println("Received body content: false" + body);
                            client.stop();
                        }
                    }
                }
            }
        }

        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            int numStations = WiFi.softAPgetStationNum();
            if (numStations > 0) {
                Logger(LN, "AP connected. Number of connected stations: %d", numStations);
            }
            previousMillis = currentMillis;
        }
    }

    Logger(LN, "Server Stop");
}

bool AutoConnect::_getWifiAuthenFromServer(String info) {
    StaticJsonDocument<96> j_wifi;
    DeserializationError error = deserializeJson(j_wifi, info);
    if (error) {
        Logger(LE, "deserialize wifi data Failed: %s", error.c_str());
        return false;
    }

    String ssid = j_wifi["message"]["SSID"];
    String password = j_wifi["message"]["PASSWORD"];

    bool result = _tryConnect(ssid, password);
    if (result) {
        return true;
    } else {
        return false;
    }
}

bool AutoConnect::_tryConnect(String ssid, String password) {
    unsigned long previousMillis = 0;
    const long interval = 500;
    int connectCount = 0;
    pWiFi.begin(ssid.c_str(), password.c_str());
    Logger(LN, "WiFi Connecting...");

    while (1) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            connectCount++;

            if (pWiFi.status() == WL_CONNECTED) {
                Logger(LN, "WiFi Connected!");
                return true;
            } else if (connectCount > 40) {
                Logger(LW, "WiFi Connected Faild!");
                return false;
            }
            previousMillis = currentMillis;
        }
    }
}