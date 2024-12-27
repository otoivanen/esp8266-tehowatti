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
const char* DEVICE_NAME = "TehoWatti";
unsigned long MQTT_RECONNECT_INTERVAL = 10000;
unsigned long SENSOR_VALUE_MIN_PUBLISH_INTERVAL = 5000;
unsigned long SENSOR_VALUE_MAX_PUBLISH_INTERVAL = 300000; // Publish sensor values every 5 mins minimum

FileManager fm; // Filemanager object for reading & writing to files
WiFiManager wm; // WiFiManager object to control WiFi connectivity
WiFiClient wiFiClient; // Initialize separate WiFi Client to interact with mqtt server on different port
PubSubClient mqttClient(wiFiClient); // Mqtt client for mqtt messaging
SensorManager sensors(ONE_WIRE_PIN); // Initialize the sensor manager
ConfigManager config(fm);
WebServerManager server(80, config); // Webserver to run on port 80 for http connections

// Declare function prototypes here to place them below loop () for better readability
void connectMqtt();
bool isPublishAllowed();
void publishSensorValues();

void setup() {

  Serial.begin(9600); // Open the serial port
  fm.begin(); // Initialize the FS
  config.loadConfig(); // Load all configs from file

  wm.setCredentials(config.getSSID(), config.getWiFiPassword(), DEVICE_NAME);
  if(!wm.connect()) {
    wm.startSoftAP();
  }

  server.begin(); // Start the web server
  mqttClient.setServer(config.getMqttServer(), config.getMqttPort());
  connectMqtt();
}

void loop() {
  wm.checkWiFiStatus(); // Ensure wifi is connected, if not start softAP for a period of time before reconnecting

  if (!mqttClient.connected()) { // Ensure mqtt broker is connected and attempt reconnect if not
    connectMqtt();
  }

  server.handleClient(); // Handle the webserver client requests
  wm.checkAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.
  sensors.readSensors();
  mqttClient.loop();
  publishSensorValues();
}

/*
Connect to mqtt broker
*/
void connectMqtt() {
  static unsigned long lastConnectionAttempt = 0;

  if(wm.status() == WL_CONNECTED && millis() - lastConnectionAttempt > MQTT_RECONNECT_INTERVAL) {

    Serial.println("Trying to establish mqtt broker connection");

    if(mqttClient.connect(DEVICE_NAME, config.getMqttUser(), config.getMqttPassword())) {
      Serial.println("Connected to mqtt broker succesfully");
    } else {
      Serial.print("Failed to connect mqtt broker with errorcode: ");
      Serial.println(mqttClient.state());
    }

    lastConnectionAttempt = millis();
  }
}

bool isPublishAllowed(unsigned long &lastPublish, float &lastTemp, float &currentTemp) {
  unsigned long fromLastPublish = millis() - lastPublish;

  if (!mqttClient.connected()) { return false; }

  if((fromLastPublish > SENSOR_VALUE_MIN_PUBLISH_INTERVAL && abs(lastTemp - currentTemp) > 0.1) || fromLastPublish > SENSOR_VALUE_MAX_PUBLISH_INTERVAL) {
    return true;
  }
  
  return false;
}

void publishSensorValues() {
  static unsigned long lastInletTempPublish = 0; // static will retain the value from last execution
  static unsigned long lastOutletTempPublish = 0;

  float inletTemp = sensors.getInletTemp();
  static float lastInletTemp = 0;
  float outletTemp = sensors.getOutletTemp();
  static float lastOutletTemp = 0;

  // Publish inletTemp
  if(isPublishAllowed(lastInletTempPublish, lastInletTemp, inletTemp)) {
    mqttClient.publish(config.getInletTempStateTopic(), String(inletTemp).c_str());
    Serial.print("Inlet temperature published: ");
    Serial.println(inletTemp);
    lastInletTempPublish = millis();
    lastInletTemp = inletTemp;
  }

  // Publish outletTemp
  if(isPublishAllowed(lastOutletTempPublish, lastOutletTemp, outletTemp)) {
    mqttClient.publish(config.getOutletTempStateTopic(), String(outletTemp).c_str());
    Serial.print("Outlet temperature published: ");
    Serial.println(outletTemp);
    lastOutletTempPublish = millis();
    lastOutletTemp = outletTemp;
  }

}