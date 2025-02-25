#include <OLEDManager.h>

OLEDManager::OLEDManager() : Adafruit_SSD1306(128, 64, &Wire, -1) {
};

/*
Method to update OLED display on row-basis only when contents change. By checking if content was changed there is no need to limit
refresh interval when calling the method due to slowness of OLED update.
*/
void OLEDManager::updateDisplay(float inletTemp, float outletTemp,const char* relayState,const char* ip, const char* mode, boolean mqtt) {
    static float lastInletTemp = -999;
    static float lastOutletTemp = -999; 
    static const char* lastRelayState = "";
    static const char* lastIp = "";
    static const char* lastMode = "";
    static bool lastMqtt = NULL;

    if (inletTemp != lastInletTemp) {
        lastInletTemp = inletTemp;
        _drawRow(1, "Inlet: " + String(inletTemp, 1) + " C");
    }

    if (outletTemp != lastOutletTemp) {
        lastOutletTemp = outletTemp;
        _drawRow(2, "Outlet: " + String(outletTemp, 1) + " C");
    }

    if (lastRelayState != relayState) {
        lastRelayState = relayState;
        _drawRow(3, "Relay: " + String(relayState));
    }

    if (lastIp != ip || lastMode != mode) {
        lastIp = ip;
        lastMode = mode;
        _drawRow(4, "IP-"+ String(mode) + ":" + String(ip));
    }

    if (lastMqtt != mqtt || lastMqtt == NULL) {
        lastMqtt = mqtt;
        _drawRow(5, mqtt ? "MQTT: Connected" : "MQTT: Disconnected");
    }
};

/*
Private method for updating the selected row. Row contents are first erased with drawing black rectangle prior to writing new content
to avoid pixel build-up.
*/
void OLEDManager::_drawRow(int row, const String& text) {
    setTextSize(1);
    setTextColor(SSD1306_WHITE);
    fillRect(0, row * 10, 128, 10, SSD1306_BLACK); // First erase contents from the row to be updated, otherwise pixels just add up
    setCursor(0, row * 10);
    print(text);
    display();
};