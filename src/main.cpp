#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>

WiFiManager wm;         // Create WiFiManager object to control WiFi functionalities
WebServerManager server(80);   // Create WebServerManager object to control the webserver functionalities

void setup() {

  Serial.begin(9600); // Open the serial port

  wm.Connect(ssid, pw); // Connect to WiFi with ssid and credentials

  server.begin(); // Start the web server
  Serial.println("Web server started");
}

void loop() {
  server.handleClient(); // Handle the webserver client requests
  wm.CheckAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}