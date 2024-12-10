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
    void CheckAPClientCount(); // Checks the softAP clientcount
    bool IsAPOn(); // Helper method to return boolean value if softAP is active

private:
    // variables for the clientcount softAP clientcount
    int _clientCount = 0;                                   // nbr of softAP clients
    unsigned long _clientCountLastCheckMillis = millis();   // when it was last time checked
};

#endif