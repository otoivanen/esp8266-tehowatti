#include <WebServerManager.h>

const char* EXPECTED_CONFIG_KEYS[] = {"ssid", "password", "mqttBroker", "mqttPort", "mqttTopic1", "mqttTopic2", "mqttRelay"};

// Constructor calls the base class constructor and takes the port and reference to filemanager as reference. Fm can then be used to store and retrieve config values.
WebServerManager::WebServerManager(uint16_t port, FileManager &fm, DallasTemperature &tempSensors) : ESP8266WebServer(port) {

    // The root route. Need to capture 'this' to be able to access a method of parent class from lambda
    on("/", HTTP_GET, [this, &fm, &tempSensors]() {
        String html = fm.readFile("/index.html");

        // Testing to print out realtime sensor value from main program
        Serial.print("Sensor1 value: ");
        Serial.println(tempSensors.getTempCByIndex(0));

        send(200, "text/html", html); // Send the config page as response
    });

    // Serve javascript file from SPIFFS
    on("/javascript", HTTP_GET, [this, &fm]() {
        String javascript = fm.readFile("/javascript.js");
        send(200, "text/html", javascript);
    });

    // CORS DEBUGGING PURPOSES
    on("/savesettings", HTTP_OPTIONS, [this]() {
        sendHeader("Access-Control-Allow-Origin", "*");
        sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        sendHeader("Access-Control-Allow-Headers", "Content-Type");
        send(204);
    });

    /*
    Route handles http request when config form is submitted. The received config is lightly validated. If invalid, error respose sent,
    if valid, config is saved to file.
    */
    on("/savesettings", HTTP_POST, [this, &fm]() {

        sendHeader("Access-Control-Allow-Origin", "*");
        sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        sendHeader("Access-Control-Allow-Headers", "Content-Type");

        if(hasArg("plain")) {
            String bodyJsonString = arg("plain"); // Get the json string from request body

            // Create json doc and deserialize body string
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, bodyJsonString);

            // If error in deserialization, send response
            if (error) {
                Serial.print("Failed to deserialize JSON: ");
                Serial.println(error.f_str());
                return send(500, "text/html", "Invalid JSON");
            }

            serializeJsonPretty(doc, Serial); // Print the received Json
            
            // Save config to file if valid
            if(!fm.saveFile("/config.json", bodyJsonString)) {
                send(500, "text/html", "Failed to save settings into FS");
            };

            // Send OK response
            send(200, "text/html", "Settings saved succesfully, device will reboot in 1sec");
            delay(1000);
            //ESP.restart();

        } else {
            send(400, "text/plain", "No config received!");
        }
    });
};

String WebServerManager::validateConfig(JsonDocument &config) {
    JsonObject obj = config.as<JsonObject>();

    for (JsonPair kv : obj) {
        Serial.println("Do this later");
    }

    return "";
};