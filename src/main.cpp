#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SensorManager.h>
#include <ConfigManager.h>
//#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Relay.h>
#include <Global_functions.h>
#include <OLEDManager.h>

const int ONE_WIRE_PIN = 16; // Pin where onewire sensors are connected
const char* DEVICE_NAME = "TehoWatti";
unsigned long MQTT_RECONNECT_INTERVAL = 10000;
unsigned long SENSOR_VALUE_MIN_PUBLISH_INTERVAL = 5000;
unsigned long SENSOR_VALUE_MAX_PUBLISH_INTERVAL = 300000; // Publish sensor values every 5 mins minimum
unsigned long RELAY_STATE_PUBLISH_INTERVAL = 300000; // Publish relay state by force every 5 mins

Adafruit_NeoPixel led(1, 15, NEO_GRB + NEO_KHZ800);

FileManager fm; // Filemanager object for reading & writing to files
WiFiManager wm; // WiFiManager object to control WiFi connectivity
WiFiClient wiFiClient; // Initialize separate WiFi Client to interact with mqtt server on different port
PubSubClient mqttClient(wiFiClient); // Mqtt client for mqtt messaging
SensorManager sensors(ONE_WIRE_PIN); // Initialize the sensor manager
ConfigManager config(fm);
Relay relay(14); // Initialize relay in output pin 14
WebServerManager server(80, config, sensors, relay); // Webserver to run on port 80 for http connections
OLEDManager oled; // Init display object



// Declare function prototypes here to place them below loop () for better readability
void connectMqtt();
bool isPublishAllowed();
void publishSensorValues();
void callback(char* topic, byte* payload, unsigned int length);
void publishRelayState();

void setup() {
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.display();
  delay(2000);
  oled.clearDisplay();
  led.begin();
  led.setPixelColor(0, led.Color(255, 255, 0));
  led.setBrightness(10);
  led.show();

  Serial.begin(9600); // Open the serial port
  fm.begin(); // Initialize the FS
  config.loadConfig(); // Load all configs from file

  wm.setCredentials(config.getSSID(), config.getWiFiPassword(), DEVICE_NAME);
  if(!wm.connect()) {
    wm.startSoftAP();
  }

  server.begin(); // Start the web server
  mqttClient.setServer(config.getMqttServer(), config.getMqttPort());
  mqttClient.setKeepAlive(60);
  mqttClient.setCallback(callback); // Assign callback function to execute when MQTT-msg is received
  connectMqtt();
}

void loop() {
  wm.checkWiFiStatus(); // Ensure wifi is connected, if not start softAP for a period of time before reconnecting

  // Ensure mqtt broker is connected and attempt reconnect if not
  if (!mqttClient.connected()) {
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

/*
Function to handle MQTT connection with restricted retry interval, and subscribe to needed topics.
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

/*
Function performs the check if sensor values are allowed to be published by calculating change in temperature
and publish min-max time intervals
*/
bool isPublishAllowed(unsigned long &lastPublish, float &lastTemp, float &currentTemp) {
  unsigned long fromLastPublish = millis() - lastPublish;

  if (!mqttClient.connected()) { return false; }

  if((fromLastPublish > SENSOR_VALUE_MIN_PUBLISH_INTERVAL && abs(lastTemp - currentTemp) > 0.1) || fromLastPublish > SENSOR_VALUE_MAX_PUBLISH_INTERVAL) {
    return true;
  }
  
  return false;
}

/*
Function handles publishing the sensor values to selected MQTT-topics if allowance criteria is met (interval & temp change)
and maintains the last reading and current reading values for comparison
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

/*
Callback function for MQTT-client that handles the incoming messages and forwards them
for further processing. Function is executed everytime a MQTT-message is received
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

/*
A Non-blocking function that forces relay state to be published with specified time intervals
in case if the state falls unknown for some reason
*/
void publishRelayState() {
  static unsigned long lastRelayStatePublish = 0;

  if(mqttClient.connected() && (millis() - lastRelayStatePublish > RELAY_STATE_PUBLISH_INTERVAL || lastRelayStatePublish == 0)) {
    mqttClient.publish(config.getRelayStateTopic(), relay.getState());
    lastRelayStatePublish = millis();
    Serial.println("Relay state published");
  }
}

/*
Global function accessible from all classes to perform all necessary operations on relay state change (publish state etc.)
Regardless of where the function was called from
*/
void setRelayState(bool state) {
  if (state) {
    relay.on();
    mqttClient.publish(config.getRelayStateTopic(), "ON", true);
  } else {
    relay.off();
    mqttClient.publish(config.getRelayStateTopic(), "OFF", true);
  } 
}