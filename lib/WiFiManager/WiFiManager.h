/*
WiFiManager class extends the native ESP8266WiFiClass, and encapsulates needed additional logics for handling 
reconnections, accespoint logic and returns states needed for displaying in the Web UI.

Author: Oskari Toivanen
Date: 2025-03-09
*/
#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>

// Inherit the default Arduino WiFiClass and create custom methods to streamline operation
class WiFiManager : public ESP8266WiFiClass {

public:
    WiFiManager();
    bool connect();
    void startSoftAP();
    void checkWiFiStatus();
    void checkAPClientCount(); // Checks the softAP clientcount
    bool isAPOn(); // Helper method to return boolean value if softAP is active
    void setCredentials(const char* ssid, const char* password, const char* deviceName);
    const char* getIP(); // Returns ip address regardless of operating mode
    const char* getMode(); // Returns STA/AP mode

private:
    const char* _ssid;
    const char* _password;
    const char* _deviceName;
    int _softAPclientCount = 0;                                   // nbr of softAP clients
    unsigned long _softAPclientCountLastCheckMillis = millis();   // when clientcount was last time checked
    unsigned long _softAPStartMillis = 0;
};

#endif