#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

FileManager fm; // Filemanager object for reading & writing to files
WiFiManager wm; // WiFiManager object to control WiFi connectivity
WebServerManager server(80, fm); // Webserver to run on port 80 for http connections
WiFiClient wiFiClient; // Initialize separate WiFi Client to interact with mqtt server on different port
PubSubClient mqttClient(wiFiClient); // Mqtt client for mqtt messaging

const char* ssid2 = ""; // Init wifi ssid
const char* pw2 = ""; // Init wifi pw

unsigned long lastmillis = millis(); // test mqtt
int i = 0;

/*
Function reads WiFi SSID and credentials from filesystem
*/
void getWiFiCredentials() {
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
    Serial.println(ssid2);
    Serial.println(pw2);
  }
}

/*
Publish mqtt message and print debug messages
*/
void connectMqtt() {
  if(wm.status() == WL_CONNECTED) {
    if(mqttClient.connect("Tehowatti", mqttUser, mqttPassword)) {
      Serial.println("Connected to mqtt broker succesfully");
    } else {
      Serial.print("Failed to connect mqtt broker with errorcode: ");
      Serial.println(mqttClient.state());

    }
  }
}

void setup() {

  Serial.begin(9600); // Open the serial port

  fm.begin(); // Initialize the FS

  getWiFiCredentials();
  wm.connect(ssid, pw); // Connect to WiFi with ssid and credentials

  server.begin(); // Start the web server

  mqttClient.setServer(mqttServer, mqttPort);
  connectMqtt();
}

void loop() {
  server.handleClient(); // Handle the webserver client requests
  wm.checkAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.

  if (millis() - lastmillis > 5000) {
    if(wm.isConnected() && mqttClient.connected()) {
      Serial.println("Trying to send mqtt");
      mqttClient.publish("test/topic", String(i).c_str());
      i++;
      lastmillis = millis();
    }
  }
}