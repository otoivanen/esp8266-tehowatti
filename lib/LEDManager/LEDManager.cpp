#include <LEDManager.h>

/**
 * @brief The constructor that extends Adafruit_NeoPixel class and predefines LED colors
 * 
 * Constructor is set up by needed parameters for WS1812B RGB-led to be controlled.
 * The led brightness and color can be adjusted. The constructor sets the predefined alert,
 * standby, relay and apMode colors as Color objects to member variables for further reference.
 */
LEDManager::LEDManager() : Adafruit_NeoPixel (1, 15, NEO_GRB + NEO_KHZ800) {
    _alertColor = Color(255, 0, 0);
    _standByColor = Color(0, 255, 0);
    _relayActivatedColor = Color(0, 0, 255);
    _apModeColor = Color(153, 102, 204);
};

/**
 * @brief Starts the led operation and does the initial brightness- and color setup
 */
void LEDManager::enable() {
    begin();
    setBrightness(10);
    setPixelColor(0, _standByColor);
    show();
}

/**
 * @brief Method (and below methods too) set the relay into predefined mode (color)
 */
void LEDManager::alert() {
    setPixelColor(0, _alertColor);
    show();
};

void LEDManager::relayActive() {
    setPixelColor(0, _relayActivatedColor);
    show();
};

void LEDManager::standby() {
    setPixelColor(0, _standByColor);
    show();
};

void LEDManager::apMode() {
    setPixelColor(0, _apModeColor);
    show();
};

/**
 * @brief Includes the LED color controls based on the device states in non-blocking manner
 * 
 * Method takes necessary states as parameters and encapsulates the logic for setting LED color
 * based on the device state. By storing and comparing current states to previous states the LED 
 * remains responsive and non-blocking (not being updated on every loop cycle)
 * 
 * @param apMode true/false whether device is in Accesspoint Mode
 * @param mqttConnection true/false whether device is connected to MQTT broker
 * @param relayState ON/OFF whehter relay is activated or deactivated
 */
void LEDManager::setStatus(bool apMode, bool mqttConnection, const char* relayState) {
    static bool lastApMode = false;
    static bool lastMqttConnection = true;
    static const char* lastRelayState = "OFF";
    static const char* lastState = "";

    if (apMode && apMode != lastApMode) {
        lastApMode = apMode;
        lastState = "apMode";
        this->apMode();
    } else if (!mqttConnection && mqttConnection != lastMqttConnection && !apMode) {
        lastMqttConnection = mqttConnection;
        lastState = "mqttConnection";
        this->alert();
    } else if (relayState == "ON" && relayState != lastRelayState && !apMode) {
        lastRelayState = relayState;
        lastState = "relayActivated";
        this->relayActive();
    } else if (lastState != "standby" && !apMode && mqttConnection && relayState != "ON") {
        lastState = "standby";
        this->standby();
    };
};