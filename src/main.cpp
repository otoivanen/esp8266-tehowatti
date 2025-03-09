/*
ESP8266-Tehowatti program controls a device that reads two DS18B20 temperature sensors, and reports the values through MQTT
to configurable topics. The program also controls a relay based on MQTT messages and/or HTTP-requests. The device can be configured and
status checked through WebUI served from the device's webserver.

Main program initializes the needed objects and performs the main loop of checking connections and connecting as needed, switching between STA/AP-modes,
handling MQTT-communication and HTTP-requests. The detailed logics of each functionality is encapsulated inside the corresponding class. Main events
of the main in addition to status checks and calling functions from other classes is to handle the MQTT-messaging. It includes also few global functions
to allow global messaging between multiple classes.

Author: Oskari Toivanen
Date: 2025-03-09
*/

#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SensorManager.h>
#include <ConfigManager.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Relay.h>
#include <Global_functions.h>
#include <OLEDManager.h>
#include <LEDManager.h>

const int ONE_WIRE_PIN = 16; // Pin where onewire sensors are connected
const char* DEVICE_NAME = "TehoWatti";
unsigned long MQTT_RECONNECT_INTERVAL = 10000;
unsigned long SENSOR_VALUE_MIN_PUBLISH_INTERVAL = 5000;
unsigned long SENSOR_VALUE_MAX_PUBLISH_INTERVAL = 300000; // Publish sensor values every 5 mins minimum
unsigned long RELAY_STATE_PUBLISH_INTERVAL = 300000; // Publish relay state by force every 5 mins

FileManager fm; // Filemanager object for reading & writing to files
WiFiManager wm; // WiFiManager object to control WiFi connectivity
WiFiClient wiFiClient; // Initialize separate WiFi Client to interact with mqtt server on different port
PubSubClient mqttClient(wiFiClient); // Mqtt client for mqtt messaging
SensorManager sensors(ONE_WIRE_PIN); // Initialize the sensor manager
ConfigManager config(fm);
Relay relay(14); // Initialize relay in output pin 14
WebServerManager server(80, config, sensors, relay); // Webserver to run on port 80 for http connections
OLEDManager oled; // Init display object
LEDManager led; // Init the led control


// Declare function prototypes here to place them below loop () for better readability
void connectMqtt();
bool isPublishAllowed();
void publishSensorValues();
void callback(char* topic, byte* payload, unsigned int length);
void publishRelayState();

void setup() {
  // Initialize and start the display
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.showBootMsg("Booting..");

  // Initialize and start the led operation, show alert color until fully booted
  led.enable();
  led.alert();

  Serial.begin(9600); // Open the serial port
  fm.begin(); // Initialize the FS
  config.loadConfig(); // Load all configs from file

  // Set the sensor addresses from config
  sensors.setInletSensorAddress(config.getInletSensorAddress());
  sensors.setOutletSensorAddress(config.getOutletSensorAddress());

  wm.setCredentials(config.getSSID(), config.getWiFiPassword(), DEVICE_NAME);
  
  oled.showBootMsg("Trying to establish WiFi Connection");
  if(!wm.connect()) {
    oled.showBootMsg("WiFi failed, starting AP");
    wm.startSoftAP();
  }

  server.begin(); // Start the web server
  mqttClient.setServer(config.getMqttServer(), config.getMqttPort());
  mqttClient.setKeepAlive(15);
  mqttClient.setCallback(callback); // Assign callback function to execute when MQTT-msg is received
  connectMqtt();
  
  oled.clearDisplay();
  oled.display();
}

void loop() {
  // Update the status led state on every cycle
  led.setStatus(wm.isAPOn(), mqttClient.connected(), relay.getState());

  // Ensure wifi is connected, if not start softAP for a period of time before reconnecting
  wm.checkWiFiStatus(); 

  // Ensure mqtt broker is connected and attempt reconnect if not
  if (!mqttClient.connected() && !wm.isAPOn()) {
    connectMqtt();
  }

  server.handleClient();    // Handle the webserver client requests
  wm.checkAPClientCount();  // Check nbr of softap clients if established. Non-blocking function.
  sensors.readSensors();    // Read the sensor values on interval specified in the sensor class
  mqttClient.loop();        // Handle MQTT-messages
  publishSensorValues();    // Publish sensor values
  publishRelayState();      // Force publish relay state in case of unknown state

  // Update the OLED -display rows if states have changed
  oled.updateDisplay(sensors.getInletTemp(), sensors.getOutletTemp(), relay.getState(), wm.getIP(), wm.getMode(), mqttClient.connected());
}

/**
 * @brief Checks MQTT-connection status and reconnects on preset interval if needed
 * 
 * If WiFi-connection is established, and time defined in MQTT_RECONNECT_INTERVAL constant
 * has passed, device retries the connection to MQTT-broker. After succesful connection the needed
 * topics are being subscribed.
 */
void connectMqtt() {
  static unsigned long lastConnectionAttempt = 0;

  if(wm.status() == WL_CONNECTED && millis() - lastConnectionAttempt > MQTT_RECONNECT_INTERVAL) {

    Serial.println("Trying to establish mqtt broker connection");

    if(mqttClient.connect(DEVICE_NAME, config.getMqttUser(), config.getMqttPassword())) {
      Serial.println("Connected to mqtt broker succesfully");
      mqttClient.subscribe(config.getRelaySetTopic()); // Subscribe to relay set topic for receiving control msg
    } else {
      Serial.print("Failed to connect mqtt broker with errorcode: ");
      Serial.println(mqttClient.state());
    }

    lastConnectionAttempt = millis();
  }
}

/**
 * @brief Determines if a sensor value is allowed to be published over MQTT
 * 
 * The function checks two conditions to allow publishing:
 * 1. If the elapsed time since last publish exceeds the minimum interval and
 * the temperature change exceeds a defined treshold (0.1c)
 * 2. If the maximum allowed publish interval has been exceeded, regardless of temperature change
 * 
 * Additionally, publishing is allowed only if the MQTT client is currently connected.A0
 * 
 * @param lastPublish the timestamp (in milliseconds) of the last succesful publish
 * @param lastTemp the last published temperature value
 * @param currentTemp the current temperature reading
 * @return true if publishing is allowed based on the condition, false otherwise
 */
bool isPublishAllowed(unsigned long &lastPublish, float &lastTemp, float &currentTemp) {
  unsigned long fromLastPublish = millis() - lastPublish;

  if (!mqttClient.connected()) { return false; }

  if((fromLastPublish > SENSOR_VALUE_MIN_PUBLISH_INTERVAL && abs(lastTemp - currentTemp) > 0.1) || fromLastPublish > SENSOR_VALUE_MAX_PUBLISH_INTERVAL) {
    return true;
  }
  
  return false;
}

/**
 * @brief Handles the publishing of the sensor values to MQTT broker.
 * 
 * The function maintains the previous temperature- and publishtime values for determining if publishing is allowed.
 * Function fetches latest readings for both sensors and publishes the values into MQTT topics defined in configs if allowed.
 */
void publishSensorValues() {
  static unsigned long lastInletTempPublish = 0; // static will retain the value from last execution
  static unsigned long lastOutletTempPublish = 0;

  char buffer[16]; // Shared buffer for publishing

  float inletTemp = sensors.getInletTemp();
  static float lastInletTemp = 0;
  float outletTemp = sensors.getOutletTemp();
  static float lastOutletTemp = 0;

  // Publish inletTemp
  if(isPublishAllowed(lastInletTempPublish, lastInletTemp, inletTemp)) {
    dtostrf(inletTemp, 6, 2, buffer); // Convert float to string
    mqttClient.publish(config.getInletTempStateTopic(), buffer);
    Serial.print("Inlet temperature published: ");
    Serial.println(inletTemp);
    lastInletTempPublish = millis();
    lastInletTemp = inletTemp;
  }

  // Publish outletTemp
  if(isPublishAllowed(lastOutletTempPublish, lastOutletTemp, outletTemp)) {
    dtostrf(outletTemp, 6, 2, buffer); // Convert float to string
    mqttClient.publish(config.getOutletTempStateTopic(), buffer);
    Serial.print("Outlet temperature published: ");
    Serial.println(outletTemp);
    lastOutletTempPublish = millis();
    lastOutletTemp = outletTemp;
  }
}

/**
 * @brief Callback function executed whenever an MQTT message is received from the broker.
 * 
 * This function is triggered automatically when a subscribed MQTT topic receives a message. 
 * It parses the incoming payload, converts it to a string, and processes the message.
 * If the message is "ON" or "OFF", it updates the relay state accordingly.
 * 
 * @param topic The MQTT topic on which the message was received.
 * @param payload The actual message data in byte format.
 * @param length The length of the incoming message payload.
 */
void callback(char* topic, byte* payload, unsigned int length) {

  char message[10]; // Buffer large enough for "ON", "OFF", and null terminator

  memcpy(message, payload, length); // Copy payload into buffer
  message[length] = '\0'; // Null-terminate the string

  Serial.println("Message received on topic: ");
  Serial.println(topic);
  Serial.println("Message: ");
  Serial.println(message);

  if (strcmp(message, "ON") == 0) {
    setRelayState(true);
  } else if (strcmp(message, "OFF") == 0) {
    setRelayState(false);
  }
}

/**
 * @brief Publishes the relaystate (on/off) with defined interval to make sure MQTT broker is synced with device status
 * 
 * The function checks if MQTT connection is established, and the defined time interval has passed between last publish.
 */
void publishRelayState() {
  static unsigned long lastRelayStatePublish = 0;

  if(mqttClient.connected() && (millis() - lastRelayStatePublish > RELAY_STATE_PUBLISH_INTERVAL || lastRelayStatePublish == 0)) {
    mqttClient.publish(config.getRelayStateTopic(), relay.getState());
    lastRelayStatePublish = millis();
    Serial.println("Relay state published");
  }
}

/**
 * @brief Global function to set relay state and perform sequence of actions related to relay statechange
 * 
 * Based on the command function sets relay activated or deactivated, and sends the state message to MQTT broker
 * which keeps other devices aware of the state. Function also switches the LED-indicator to show blue / standby
 * based on state changes. Being a global function it can be accessed from any class and thus the relay, LED and
 * MQTT message are sent regardless if state change was requested through HTTP-request or MQTT message.
 * 
 * @param state true switches the relay active, false deactivates the relay
 */
void setRelayState(bool state) {
  if (state) {
    relay.on();
    mqttClient.publish(config.getRelayStateTopic(), "ON", true);
    led.relayActive();
  } else {
    relay.off();
    mqttClient.publish(config.getRelayStateTopic(), "OFF", true);
    led.standby();
  } 
}

/**
 * @brief Function returns the JSON-string containing all necessary states of the device for displaying in UI
 * 
 * Function checks the WiFi state (AP/STA) and gets the SSID from wifi controller. Then JSON
 * object is then formed by getting the states and configs from all necessary classes. Function is defined as
 * global to be accessible from WebServer class, and still being able to gather states from all objects
 * 
 * @return JSON-string containing necessary configs and states for WebUI
 */
String getStatesAsJson() {
  JsonDocument doc;

  String currentSSID;

  if (wm.getMode() == "STA") {
    currentSSID = wm.SSID();
  } else {
    currentSSID = wm.softAPSSID();
  }

  // Populate JSON document with current config values
  doc["wifiMode"] = wm.getMode();
  doc["wifiIP"] = wm.getIP();
  doc["SSID"] = currentSSID;
  doc["MQTTConnected"] = mqttClient.connected();
  doc["MQTTBroker"] = config.getMqttServer().toString();
  doc["MQTTPort"] = config.getMqttPort();
  doc["InletTemp"] = sensors.getInletTemp();
  doc["OutletTemp"] = sensors.getOutletTemp();
  doc["RelayState"] = relay.getState();
  doc["inletSensorAddress"] = sensors.getInletSensorAddress();
  doc["outletSensorAddress"] = sensors.getOutletSensorAddress();

  // Serialize JSON document into a string
  String serializedStates;
  serializeJson(doc, serializedStates);

  return serializedStates;
}
