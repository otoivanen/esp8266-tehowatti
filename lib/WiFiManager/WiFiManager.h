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

private:
    const char* _ssid;
    const char* _password;
    const char* _deviceName;
    int _softAPclientCount = 0;                                   // nbr of softAP clients
    unsigned long _softAPclientCountLastCheckMillis = millis();   // when clientcount was last time checked
    unsigned long _softAPStartMillis = 0;
};

#endif