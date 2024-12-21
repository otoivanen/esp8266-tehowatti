#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>

// Inherit the default Arduino WiFiClass and create custom methods to streamline operation
class WiFiManager : public ESP8266WiFiClass {

public:
    WiFiManager();
    void setCredentials(const char* ssid, const char* password);
    void connect();
    void disconnect();
    void printStatus();
    void checkAPClientCount(); // Checks the softAP clientcount
    bool isAPOn(); // Helper method to return boolean value if softAP is active

private:
    const char* _ssid;
    const char* _password;
    // variables for the clientcount softAP clientcount
    int _clientCount = 0;                                   // nbr of softAP clients
    unsigned long _clientCountLastCheckMillis = millis();   // when clientcount was last time checked
};

#endif