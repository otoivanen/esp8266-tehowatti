#include "WiFiManager.h"

const int MAX_ATTEMPTS = 10;        // The number of max consecutive wifi connection attempts
const int DELAY_ATTEMPT = 1000;     // Delay between connection attempts

// Accesspoint constants
const char* AP_SSID = "TehoWatti";

// Constructor
WiFiManager::WiFiManager () {
    
}

// Tries to connect to WiFi with given credentials and ssid
void WiFiManager::Connect(const char* ssid, const char* password) {

    begin(ssid, password);

    int attempt = 1;

    while (status() != WL_CONNECTED && attempt-1 < MAX_ATTEMPTS) {
        Serial.print("Connection attempt: ");
        Serial.println(attempt);
        attempt++;
        delay(DELAY_ATTEMPT);
    }

    if (status() == WL_CONNECTED) {
        Serial.println("WiFi connection established");
    } else {
        Serial.println("Wifi connection failed to establish. Starting accesspoint");
        
        // Try to start softAP and report the result
        Serial.println(softAP(AP_SSID) ? "Accesspoint ready" : "Accesspoint failed");

        Serial.print("Soft-AP IP address = ");
        Serial.println(softAPIP());
    }
}

// Disconnect from WiFi
void WiFiManager::Disconnect() {
    disconnect();
    Serial.println("Disconnected from WiFi");
}

// Print SSID and IP-address if connected
void WiFiManager::PrintStatus() {
    if (status() == WL_CONNECTED) {
        Serial.print("Connected to: ");
        Serial.println(SSID());
        Serial.print("IP Address: ");
        Serial.println(localIP());
    } else {
        Serial.println("Not connected to any WiFi.");
    }
}