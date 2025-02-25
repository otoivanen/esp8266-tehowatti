/*
Custom class that inherits the ESP8266WebServer baseclass. Custom class extends the baseclass with modified constructor
to be able to receive references into other objects managing hardware etc. This is crucial for interacting with hardware through HTTP-requests
in addition to MQTT-messages in parallel. The HTTP-routes are defined in this custom class constructor to keep them away from main program
for better readability and maintainability

Date: 2025-02-25
Author: Oskari Toivanen
*/

#ifndef WebServerManager_h
#define WebServerManager_h

#include <ESP8266WebServer.h>
#include <ConfigManager.h>
#include <SensorManager.h>
#include <Relay.h>
#include <ArduinoJson.h>
#include <Global_functions.h>

// Inherit the default Arduino WiFiClass and create custom methods to streamline operation
class WebServerManager : public ESP8266WebServer {

public:
    // Constructor will call the ESP8266WebServer constructor and requires a port
    WebServerManager(uint16_t port, ConfigManager &config, SensorManager &sensors, Relay &relay);

private:
    void _streamFile(const char * path);
};

#endif