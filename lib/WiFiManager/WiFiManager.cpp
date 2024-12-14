#include "WiFiManager.h"

const int MAX_ATTEMPTS = 10; // The number of max consecutive wifi connection attempts
const int DELAY_ATTEMPT = 5000; // Delay between connection attempts ms
const char* AP_SSID = "TehoWatti"; // Name of the softAP
const long CLIENT_COUNT_INTERVAL = 1000; // Interval between checking softAP client counts ms

// Constructor
WiFiManager::WiFiManager () {
    
}

// Attempts to connect to given SSID, if fails, starts softAP to enable connectivity
void WiFiManager::connect(const char* ssid, const char* password) {

    Serial.println("Attempting to establish WiFi connection");

    begin(ssid, password);

    int attempt = 1;

    while (status() != WL_CONNECTED && attempt < MAX_ATTEMPTS) {
        Serial.print("Connection attempt: ");
        Serial.println(attempt);
        attempt++;
        delay(DELAY_ATTEMPT);
    }

    if (status() == WL_CONNECTED) {
        Serial.println("WiFi connection established");
        Serial.println(localIP());
    } else {
        Serial.println("WiFi connection failed. Starting access point...");

        // Try to start softAP and report the result
        if (softAP(AP_SSID)) {
            Serial.println("Access point ready.");
            Serial.print("Soft-AP IP address = ");
            Serial.println(softAPIP());
        } else {
            Serial.println("Failed to start Access Point.");
        }
    }
}

// Disconnect from WiFi
void WiFiManager::disconnect() {
    disconnect();
    Serial.println("Disconnected from WiFi");
}

// Print SSID and IP-address if connected
void WiFiManager::printStatus() {
    if (status() == WL_CONNECTED) {
        Serial.print("Connected to: ");
        Serial.println(SSID());
        Serial.print("IP Address: ");
        Serial.println(localIP());
    } else {
        Serial.println("Not connected to any WiFi.");
    }
}

// Check the number of connected softAP clients in specified interval and report to Serial if changes
void WiFiManager::checkAPClientCount() {

    // Run only if softAP is on and interval has passed
    if (isAPOn() && millis() - _clientCountLastCheckMillis > CLIENT_COUNT_INTERVAL) {
        _clientCountLastCheckMillis = millis(); // set the timestamp for last check
        
        int currentClientCount = softAPgetStationNum();

        if (currentClientCount != _clientCount) {
            Serial.print("Connected SoftAP Clients: ");
            Serial.println(currentClientCount);
            _clientCount = currentClientCount;
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