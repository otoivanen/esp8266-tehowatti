#include <LEDManager.h>

LEDManager::LEDManager() : Adafruit_NeoPixel (1, 15, NEO_GRB + NEO_KHZ800) {
    _alertColor = Color(255, 0, 0);
    _standByColor = Color(0, 255, 0);
    _relayActivatedColor = Color(0, 0, 255);
    _apModeColor = Color(153, 102, 204);
};

void LEDManager::enable() {
    begin();
    setBrightness(10);
    setPixelColor(0, _standByColor);
    show();
}

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

/*
Control the led based on device states in non-blocking manner by checking changes in variables before 
sending commands to the led itself. Device can have three separate states.
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
    } else if (!mqttConnection && mqttConnection != lastMqttConnection) {
        lastMqttConnection = mqttConnection;
        lastState = "mqttConnection";
        this->alert();
    } else if (relayState == "ON" && relayState != lastRelayState) {
        lastRelayState = relayState;
        lastState = "relayActivated";
        this->relayActive();
    } else if (lastState != "standby" && !apMode && mqttConnection && relayState != "ON") {
        lastState = "standby";
        this->standby();
    };
};