#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SensorManager.h>
#include <ConfigManager.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

const int ONE_WIRE_PIN = 16; // Pin where onewire sensors are connected
const char* DEVICE_NAME = "TehoWatti";
unsigned long MQTT_RECONNECT_INTERVAL = 10000;
unsigned long SENSOR_VALUE_MIN_PUBLISH_INTERVAL = 5000;
unsigned long SENSOR_VALUE_MAX_PUBLISH_INTERVAL = 300000; // Publish sensor values every 5 mins minimum

// Test display and led
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_NeoPixel led(1, 15, NEO_GRB + NEO_KHZ800);

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

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
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
  connectMqtt();

  // Initial test for relay, remove once logic is done
  int relaypin = 14;
  pinMode(relaypin, OUTPUT);
  digitalWrite(relaypin, LOW);
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