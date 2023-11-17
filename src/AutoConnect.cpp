#include "AutoConnect.h"

AutoConnect *AutoConnect::instance = nullptr;
AutoConnect::AutoConnect(WiFiClass &WiFi) : pWiFi(WiFi) {
    instance = this;
    pWiFi.onEvent(_wifiEvent);
}

void AutoConnect::begin() {

    StaticJsonDocument<96> j_wifi = _getWifiAuthen();
    if (!j_wifi.isNull()) {
        String _ssid = j_wifi["SSID"].as<String>();
        String _pass = j_wifi["PASSWORD"].as<String>();
        pWiFi.begin(_ssid.c_str(), _pass.c_str());
        pWiFi.mode(WIFI_STA);
    } else {
        if (pWiFi.getMode() == WIFI_STA) {
            _beginServer();
        }
    }
}

StaticJsonDocument<96> AutoConnect::_getWifiAuthen() {
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
        Serial.println(instance->pWiFi.SSID());
        Serial.println(instance->pWiFi.psk());
        Serial.println(instance->pWiFi.localIP());
        Serial.println(instance->pWiFi.RSSI());
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
    Serial.println("beginServer");
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
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    while (1) {
        WiFiClient client = server.available();
        if (client) {
            while (client.connected()) {
                if (client.available()) {
                    String data = client.readStringUntil('\n');
                    Serial.println("Received data: " + data);
                    if (data == "\r") {
                        Serial.println("Reading body:");
                        String body = "";
                        while (client.available()) {
                            char c = client.read();
                            Serial.print(c);
                            body += c;
                        }

                        bool result = _getWifiAuthenFromServer(body);
                        Serial.println("Client disconnected");
                        if (result) {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Connection: close");
                            client.println("Content-Type: text/plain");
                            client.println();
                            client.println("Received body content: true" + body);
                            client.stop();
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
                Serial.print("AP connected. Number of connected stations: ");
                Serial.println(numStations);
            }
            previousMillis = currentMillis;
        }
    }

    Serial.println("\nServer Stop");
}

bool AutoConnect::_getWifiAuthenFromServer(String info) {
    StaticJsonDocument<96> j_wifi;
    DeserializationError error = deserializeJson(j_wifi, info);
    if (error) {
        Serial.printf("deserialize wifi data Failed: %s\n", error.c_str());
        return false;
    }

    String ssid = j_wifi["message"]["SSID"];
    String password = j_wifi["message"]["PASSWORD"];

    Serial.printf("\nSSID: %s  PASSWORD: %s\n", ssid, password);
    Serial.println("WIFI data load complete\n");

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
    Serial.print("WiFi Connecting...");

    while (1) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            connectCount++;
            Serial.print(".");
            if (pWiFi.status() == WL_CONNECTED) {
                Serial.println("\nWiFi Connected!");
                return true;
            } else if (connectCount > 40) {
                Serial.println("\nWiFi Connected Faild!");
                return false;
            }
            previousMillis = currentMillis;
        }
    }
}