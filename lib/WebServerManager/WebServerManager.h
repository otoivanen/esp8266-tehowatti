#ifndef WebServerManager_h
#define WebServerManager_h

#include <ESP8266WebServer.h>
#include <ConfigManager.h>
#include <ArduinoJson.h>

// Inherit the default Arduino WiFiClass and create custom methods to streamline operation
class WebServerManager : public ESP8266WebServer {

public:
    // Constructor will call the ESP8266WebServer constructor and requires a port
    WebServerManager(uint16_t port, FileManager &fm, ConfigManager &config);
};

#endif