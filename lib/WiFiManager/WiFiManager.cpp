#include "WiFiManager.h"

const int CONNECTION_MAX_ATTEMPTS = 5; // The number of max consecutive wifi connection attempts
const int CONNECTION_ATTEMPT_DELAY = 5000; // Delay between connection attempts ms
const int SOFTAP_CLIENT_COUNT_INTERVAL = 1000; // Interval between checking softAP client counts ms
const unsigned int SOFTAP_TIMEOUT = 60000; // Timeout for shutting softAP and retrying connect


// Constructor
WiFiManager::WiFiManager () {
    
}

/*
Establish WiFi connection. If succesful, set autoReconnect in case of connection failure when program is running.
If initial connection fails, and AP start given as parameter, will start softAP. Without startAP = true works as
for reconnecting without starting AP.

Params:
    - const char* ssid - WiFi SSID
    - const char* password - WiFi password
    - bool startAP - whether to start softAP if connection fails
*/
bool WiFiManager::connect() {

    Serial.println("Attempting to establish WiFi connection");

    begin(_ssid, _password);
    

    int attempt = 1;

    while (status() != WL_CONNECTED && attempt < CONNECTION_MAX_ATTEMPTS) {
        Serial.print("Connection attempt: ");
        Serial.println(attempt);
        attempt++;
        delay(CONNECTION_ATTEMPT_DELAY);
    }

    if (status() == WL_CONNECTED) {
        Serial.println("WiFi connection established");
        Serial.println(localIP());
        return true;
    } else {
        return false;
    }
}

void WiFiManager::startSoftAP() {
        Serial.println("Starting WiFi accesspoint......");

        // Try to start softAP and report the result
        if (softAP(_deviceName)) {
            Serial.println("Access point ready.");
            Serial.print("Soft-AP IP address = ");
            Serial.println(softAPIP());
            _softAPStartMillis = millis();
        } else {
            Serial.println("Failed to start Access Point.");
        }
}

/*
*   Check WiFi status periodically and if not connected, switch between AP mode and STA mode until connection is established
*/
void WiFiManager::checkWiFiStatus() {
    if (status() != WL_CONNECTED) {
        if (isAPOn() && millis() - _softAPStartMillis > SOFTAP_TIMEOUT && softAPgetStationNum() == 0) {
            Serial.println("Switching from AP mode to STA mode..");
            softAPdisconnect(true);
            connect();
        } else if (!isAPOn()) {
            Serial.println("Switching from STA mode to AP mode..");
            startSoftAP();
        }
    }
}

// Check the number of connected softAP clients in specified interval and report to Serial if changes
void WiFiManager::checkAPClientCount() {

    // Run only if softAP is on and interval has passed
    if (isAPOn() && millis() - _softAPclientCountLastCheckMillis > SOFTAP_CLIENT_COUNT_INTERVAL) {
        _softAPclientCountLastCheckMillis = millis(); // set the timestamp for last check
        
        int currentClientCount = softAPgetStationNum();

        if (currentClientCount != _softAPclientCount) {
            Serial.print("Connected SoftAP Clients: ");
            Serial.println(currentClientCount);
            _softAPclientCount = currentClientCount;
        }
    }
}

// Helper function to return boolean value whether softAP is on or not
bool WiFiManager::isAPOn() {
    if(softAPIP() != IPAddress(0, 0, 0, 0)) {
        return true;
    } else {
        return false;
    }
}

// Getters and setters
void WiFiManager::setCredentials(const char* ssid, const char* password, const char* deviceName) {
    _ssid = ssid;
    _password = password;
    _deviceName = deviceName;
}