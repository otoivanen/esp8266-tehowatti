#include <OLEDManager.h>

/**
 * @brief Constructor extends the adafruit_SSD1306 class
 */
OLEDManager::OLEDManager() : Adafruit_SSD1306(128, 64, &Wire, -1) {
};

/**
 * @brief Updates the OLED display row-by-row in non-blocking manner to keep program responsive
 * 
 * The method received current states of variables shown in OLED and maintaines their previous states. Only if 
 * a variable is changed from it's previous state, the rows with changed content is being updated
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

/**
 * @brief Private method to draw single row in the OLED display
 * 
 * Method draws one row at a time and only when needed to avoid overhead of writing to OLED. It receives the 
 * row number and content to write as parameter. First the existing contents are overwritten by drawing 
 * black reactangle and then new content is written and display() is called to actually show the content.
 * 
 * @param row The rownumber to rewrite
 * @param text The text content to write
 */
void OLEDManager::_drawRow(int row, const String& text) {
    setTextSize(1);
    setTextColor(SSD1306_WHITE);
    fillRect(0, row * 10, 128, 10, SSD1306_BLACK); // First erase contents from the row to be updated, otherwise pixels just add up
    setCursor(0, row * 10);
    print(text);
    display();
};

/**
 * @brief Method that allows to show custom text as needed in OLED display, eg. bootupstatus
 * 
 * Method clears the OLED display and writes the given text
 * 
 * @param text The content to write into OLED
 */
void OLEDManager::showBootMsg(const char* text) {
    clearDisplay();
    setTextColor(SSD1306_WHITE);
    setTextSize(1);
    setCursor(0,0);
    print(text);
    display();
}