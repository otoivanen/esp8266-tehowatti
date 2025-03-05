/*
Class that extends the Adafruit_SSD1306 library for I2C -oled displays. Extended class is used to encapsulate the logics needed for 
OLED update on row basis only when contents change, to avoid using time intervals.

Date: 2025-02-25
Author: Oskari Toivanen
*/

#ifndef OLED_MANAGER_H
#define OLED_MANAGER_H

#include <Wire.h>
#include <Adafruit_SSD1306.h>

class OLEDManager : public Adafruit_SSD1306 {
public:
    OLEDManager();
    void updateDisplay(float inletTemp, float outletTemp, const char* relayState, const char* ip, const char* mode, boolean mqtt);
    void showBootMsg(const char* text);
private:
    void _drawRow(int row, const String& text);
};

#endif