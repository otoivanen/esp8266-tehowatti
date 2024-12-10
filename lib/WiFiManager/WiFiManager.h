#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>

// Inherit the default Arduino WiFiClass and create custom methods to streamline operation
class WiFiManager : public ESP8266WiFiClass {

public:
    WiFiManager();
    void Connect(const char* ssid, const char* password);
    void Disconnect();
    void PrintStatus();
};

#endif