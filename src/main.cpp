#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>

FileManager fm; // Initialize the filemanager
WiFiManager wm; // Initialize WiFiManager object to control WiFi functionalities
WebServerManager server(80, fm); // Initialize WebServerManager object to control the webserver functionalities, pass filemanager reference to operate files from webserver class

void setup() {

  Serial.begin(9600); // Open the serial port

  wm.connect(ssid, pw); // Connect to WiFi with ssid and credentials

  server.begin(); // Start the web server
  Serial.println("Web server started");


}

void loop() {
  server.handleClient(); // Handle the webserver client requests
  wm.checkAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.
}