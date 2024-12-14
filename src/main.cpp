#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>

FileManager fm; // Initialize the filemanager
WiFiManager wm; // Initialize WiFiManager object to control WiFi functionalities
WebServerManager server(80, fm); // Initialize WebServerManager object to control the webserver functionalities, pass filemanager reference to operate files from webserver class

const char* ssid2 = ""; // Init wifi ssid
const char* pw2 = ""; // Init wifi pw

void setup() {

  Serial.begin(9600); // Open the serial port
  fm.begin(); // Initialize the FS

  // Read config from file and deserialize into json doc
  String configString = fm.readFile("/config.json");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configString);

  // Verify deserialization errors and find ssid and password
  if(error) {
    Serial.print("Failed to deserialize config JSON: ");
    Serial.println(error.f_str());
  } else if (doc.containsKey("ssid") && doc.containsKey("password")) {
    ssid2 = doc["ssid"];
    pw2 = doc["password"];
  }

  wm.connect(ssid2, pw2); // Connect to WiFi with ssid and credentials
  server.begin(); // Start the web server

}

void loop() {
  server.handleClient(); // Handle the webserver client requests
  wm.checkAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.
}