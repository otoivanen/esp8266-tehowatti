#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerManager.h>
#include <secrets.h>
#include <FileManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int ONE_WIRE_PIN = 4;

FileManager fm; // Filemanager object for reading & writing to files
WiFiManager wm; // WiFiManager object to control WiFi connectivity
WiFiClient wiFiClient; // Initialize separate WiFi Client to interact with mqtt server on different port
PubSubClient mqttClient(wiFiClient); // Mqtt client for mqtt messaging
OneWire oneWire(ONE_WIRE_PIN); // Create onewire driver on the pin 4
DallasTemperature tempSensors(&oneWire); // Pass the reference to onewire pin for DallasTemperature object
WebServerManager server(80, fm, tempSensors); // Webserver to run on port 80 for http connections

const char* ssid2 = ""; // Init wifi ssid
const char* pw2 = ""; // Init wifi pw
int i = 0;

// Declare function prototypes here to place them below loop () for better readability
void getWiFiCredentials();
void connectMqtt();
void readSensors();
bool isSensorPublishAllowed(unsigned long lastPublishMillis, float lastValue, float currentValue);
void publishSensors();

float inletTemp = 0;
float lastInletTemp = 0;
float outletTemp = 0;
float lastOutletTemp = 0;
const float reportTempTreshold = 0.5;

const int sensorReadInterval = 5000;  // How often sensors are read
unsigned long lastSensorReadMillis = millis();
const int sensorPublishMaxInterval = 1800000; // Publish atleast every 30 minutes, msec
unsigned long lastInletTempPublishMillis = millis(); // Store when the sensorvalues were last published
unsigned long lastOutletTempPublishMillis = millis(); // Store when the sensorvalues were last published



void setup() {

  Serial.begin(9600); // Open the serial port

  fm.begin(); // Initialize the FS

  getWiFiCredentials();
  wm.connect(ssid, pw); // Connect to WiFi with ssid and credentials

  server.begin(); // Start the web server

  mqttClient.setServer(mqttServer, mqttPort);
  connectMqtt();

  tempSensors.begin();
}

void loop() {
  server.handleClient(); // Handle the webserver client requests
  wm.checkAPClientCount(); // Check nbr of softap clients if established. Non-blocking function.
  readSensors();
  publishSensors(); // Read and publish sensor values if allowed
}

/*
Function reads WiFi SSID and credentials from filesystem
*/
void getWiFiCredentials() {
   // Read config from file and deserialize into json doc
  String configString = fm.readFile("/config.json");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configString);

  // Verify deserialization errors and find ssid and password ### Change this to copy the value instead of assigning into json doc
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
Connect to mqtt broker
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

/*
* Read the sensor values and store in global variables
*/
void readSensors() {
  if (millis() - lastSensorReadMillis > sensorReadInterval) {
    tempSensors.requestTemperatures();
    inletTemp = tempSensors.getTempCByIndex(0);
    outletTemp = tempSensors.getTempCByIndex(1);
    lastSensorReadMillis = millis();

    Serial.println(String(inletTemp).c_str());
    Serial.println(String(outletTemp).c_str());
  }
}

/*
* Check if the sensor publish is allowed based on the time intervals and value changes more than defined treshold
*/
bool isSensorPublishAllowed(unsigned long lastPublishMillis, float lastValue, float currentValue) {
  return (abs(lastValue - currentValue) > reportTempTreshold || millis() - lastPublishMillis > sensorPublishMaxInterval);
}

/*
* Publish both sensor values to respective topics if publishing is allowed
*/
void publishSensors() {

    if(isSensorPublishAllowed(lastInletTempPublishMillis, lastInletTemp, inletTemp)) {
      mqttClient.publish(inletTempStateTopic, String(inletTemp).c_str());
      lastInletTempPublishMillis = millis();
      Serial.println("Inlet temp published");
    }

    if(isSensorPublishAllowed(lastOutletTempPublishMillis, lastOutletTemp, outletTemp)) {
      mqttClient.publish(outletTempStateTopic, String(outletTemp).c_str());
      lastOutletTempPublishMillis = millis();
      Serial.println("Outlet temp published");
    }

    lastInletTemp = inletTemp;
    lastOutletTemp = outletTemp;
  }