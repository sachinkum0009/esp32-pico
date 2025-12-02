#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password);
    bool connect();
    bool isConnected();
    void printStatus();

private:
    const char* _ssid;
    const char* _password;
};

#endif // WIFI_MANAGER_H
