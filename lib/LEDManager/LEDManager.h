/*
Class that extends the Adafruit_NeoPixel class for controlling addressable RGB-leds. Extended class contains method for maintaining
the led status that reflects the current device status by checking connections and relaystate.

Date: 2025-02-25
Author: Oskari Toivanen
*/

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LEDManager : Adafruit_NeoPixel {
public:
    LEDManager();
    void standby();
    void alert();
    void relayActive();
    void apMode();
    void enable();
    void setStatus(bool apMode, bool mqttConnection, const char* relayState);
    const char* getStatus();

private:
    uint32_t _standByColor;
    uint32_t _alertColor;
    uint32_t _relayActivatedColor;
    uint32_t _apModeColor;
    const char* _status;
};

#endif