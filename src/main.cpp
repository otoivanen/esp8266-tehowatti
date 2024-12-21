#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SensorManager.h>
#include <ConfigManager.h>

const int ONE_WIRE_PIN = 4; // Pin where onewire sensors are connected

FileManager fm; // Filemanager object for reading & writing to files
WiFiManager wm; // WiFiManager object to control WiFi connectivity
WiFiClient wiFiClient; // Initialize separate WiFi Client to interact with mqtt server on different port
PubSubClient mqttClient(wiFiClient); // Mqtt client for mqtt messaging
SensorManager sensors(ONE_WIRE_PIN); // Initialize the sensor manager
ConfigManager config(fm);
WebServerManager server(80, fm, config); // Webserver to run on port 80 for http connections

// Declare function prototypes here to place them below loop () for better readability
void connectMqtt();

void setup() {

  Serial.begin(9600); // Open the serial port
  fm.begin(); // Initialize the FS
  config.loadConfig(); // Load all configs from file
  wm.setCredentials(config.getSSID(), config.getWiFiPassword());
  //wm.setCredentials(ssid, pw);
  wm.connect(); // Connect to WiFi with ssid and credentials
  server.begin(); // Start the web server
  mqttClient.setServer(config.getMqttServer(), config.getMqttPort());
  connectMqtt();
}

void loop() {
  if (!mqttClient.connected()) {
    //connectMqtt();
  }

  server.handleClient(); // Handle the webserver client requests
  wm.checkAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.
  sensors.readSensors();
  mqttClient.loop();
}

/*
Connect to mqtt broker
*/
void connectMqtt() {
  if(wm.status() == WL_CONNECTED) {
    if(mqttClient.connect("Tehowatti", config.getMqttUser(), config.getMqttPassword())) {
      Serial.println("Connected to mqtt broker succesfully");
    } else {
      Serial.print("Failed to connect mqtt broker with errorcode: ");
      Serial.println(mqttClient.state());
    }
  }
}