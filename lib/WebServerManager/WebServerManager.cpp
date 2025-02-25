#include <WebServerManager.h>

/*
Constructor calls the base class constructor and takes the port and reference to filemanager, sensors and relay for accessing the other classes states.
*/
WebServerManager::WebServerManager(uint16_t port, ConfigManager &config, SensorManager &sensors, Relay &relay) : ESP8266WebServer(port) {

    /*
    Serve the root route html from SPIFFS memory
    */
    on("/", HTTP_GET, [this]() {
        _streamFile("/index.html");
    });

    /*
    Serve the javascript file as stream from SPIFFS memory
    */
    on("/javascript", HTTP_GET, [this]() {
        _streamFile("/javascript.js");
    });

    /*
    Set CORS -options for savesettings route, needed while debugging to if making requests from different host than device itself
    */
    on("/savesettings", HTTP_OPTIONS, [this]() {
        sendHeader("Access-Control-Allow-Origin", "*");
        sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        sendHeader("Access-Control-Allow-Headers", "Content-Type");
        send(204);
    });

    /*
    Route handles http request when config form is submitted. The received config is lightly validated with configmanager's setters. 
    If invalid, error respose sent, if valid, config is saved to file.
    */
    on("/savesettings", HTTP_POST, [this, &config]() {

        sendHeader("Access-Control-Allow-Origin", "*");
        sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        sendHeader("Access-Control-Allow-Headers", "Content-Type");

        if(hasArg("plain")) {
            String body = arg("plain"); // Get the json string from request body

            bool validInputs = true; // ConfigManager setters validate the inputs lightly

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, body);

            // If error in deserialization, send response
            if (error) {
                Serial.print("Failed to deserialize JSON: ");
                Serial.println(error.f_str());
                return send(500, "text/html", "Invalid JSON");
            }

            serializeJsonPretty(doc, Serial); // Print the received Json

            if (doc.containsKey("ssid")) { validInputs &= config.setSSID(doc["ssid"]); }
            if (doc.containsKey("password")) { validInputs &= config.setWiFiPassword(doc["password"]); }
            if (doc.containsKey("mqttServer")) { validInputs &= config.setMqttServer(doc["mqttServer"]); }
            if (doc.containsKey("mqttPort")) { validInputs &= config.setMqttPort(doc["mqttPort"]); }
            if (doc.containsKey("mqttUser")) { validInputs &= config.setMqttUser(doc["mqttUser"]); }
            if (doc.containsKey("mqttPassword")) { validInputs &= config.setMqttPassword(doc["mqttPassword"]); }
            if (doc.containsKey("inletTempStateTopic")) { validInputs &= config.setInletTempStateTopic(doc["inletTempStateTopic"]); }
            if (doc.containsKey("outletTempStateTopic")) { validInputs &= config.setOutletTempStateTopic(doc["outletTempStateTopic"]); }
            if (doc.containsKey("relayStateTopic")) { validInputs &= config.setRelayStateTopic(doc["relayStateTopic"]); }
            if (doc.containsKey("relaySetTopic")) { validInputs &= config.setRelaySetTopic(doc["relaySetTopic"]); }

            if(!validInputs) { send(500, "text/html", "Some inputs were invalid, check empty inputs and formats"); }

            config.saveConfig();

            // Send OK response
            send(200, "text/html", "Settings saved succesfully, device will reboot in 1sec");
            delay(1000);
            ESP.restart();

        } else {
            send(400, "text/plain", "No config received!");
        }
    });

    /*
    Route for actuating the Relay and updating related states via Web UI by receiving ON/OFF commands as query parameters.
    If no parameters are given the current relay state is returned.
    */
   on("/relay", HTTP_GET, [this, &relay]() {
    if(hasArg("state")) {
        String state = arg("state"); // Store the state value from query params
        
        Serial.print("Received relay state request from WebUI: ");
        Serial.println(state);

        if (state == "ON") {
            setRelayState(true);
        } else if (state == "OFF") {
            setRelayState(false);
        }

        send(200, "text/html", "Relay state updated");
    } else {
        send(200, "text/html", relay.getState());
    }
   });
}

/*
Private method to stream files from SPIFFS memory. Streaming is crucial here, not to load whole file into RAM 
but instead sending it bit by bit
*/
void WebServerManager::_streamFile(const char* path) {
    File file = LittleFS.open(path, "r");
        if (!file) {
            send(404, "text/plain", "File not found");
        }

        size_t sent = streamFile(file, "text/html");
    file.close();
}