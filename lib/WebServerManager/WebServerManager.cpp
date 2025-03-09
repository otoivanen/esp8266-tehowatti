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
    Serve the stylesheet
    */
    on("/stylesheet", HTTP_GET, [this]() {
        _streamFile("/style.css", "text/css");
    });

    /*
    Serve the HTML fragments for maincontainer in index.html
    */
    on("/status", HTTP_GET, [this]() {
    _streamFile("/status.html");
    });

   on("/mqttconfig", HTTP_GET, [this]() {
        _streamFile("/mqttconfig.html");
   });

   on("/wificonfig", HTTP_GET, [this]() {
        _streamFile("/wificonfig.html");
   });

   on("/sensorconfig", HTTP_GET, [this]() {
        _streamFile("/sensorconfig.html");
   });

   /*
   Set global preflight cors options
   */
    onNotFound([&]() {
        if (method() == HTTP_OPTIONS) {
            sendHeader("Access-Control-Allow-Origin", "*");
            sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            sendHeader("Access-Control-Allow-Headers", "Content-Type");
            send(204);
        }
    });

    /*
    GET route for fetching configs as json from the configManager through HTTP-request
    */
    on("/settings", HTTP_GET, [this, &config]() {

        sendHeader("Access-Control-Allow-Origin", "*");
        sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        sendHeader("Access-Control-Allow-Headers", "Content-Type");

        // Get the configs as json string
        String jsonResponse = config.getConfigAsJson();

        send(200, "application/json", jsonResponse);
    });

    /*
    POST route handles http request when config form is submitted. The received config is lightly validated with configmanager's setters. 
    If invalid, error respose sent, if valid, config is saved to file.
    */
    on("/settings", HTTP_POST, [this, &config]() {

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
            if (doc.containsKey("inletSensorAddress")) { validInputs &= config.setInletSensorAddress(doc["inletSensorAddress"]); }
            if (doc.containsKey("outletSensorAddress")) { validInputs &= config.setOutletSensorAddress(doc["outletSensorAddress"]); }

            if(!validInputs) {
                send(500, "text/html", "Some inputs were invalid, check empty inputs and formats"); 
                return;
            }

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
    GET route for actuating the Relay and updating related states via Web UI by receiving ON/OFF commands as query parameters.
    If no parameters are given the current relay state is returned. Route calls global function in main program to
    perform sequence of actions across multiple classes
    */
   on("/relay", HTTP_GET, [this, &relay]() {

    sendHeader("Access-Control-Allow-Origin", "*");
    sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    sendHeader("Access-Control-Allow-Headers", "Content-Type");

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

   /*
   GET route to restart the device
   */
   on("/restart", HTTP_GET, [this]() {

    sendHeader("Access-Control-Allow-Origin", "*");
    sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    sendHeader("Access-Control-Allow-Headers", "Content-Type");

    send(200, "text/html", "Device is now restarting");
    delay(1000);
    ESP.restart();
   });

   /*
   GET route to fetch device states as json
   */
  on("/states", HTTP_GET, [this]() {
    sendHeader("Access-Control-Allow-Origin", "*");
    sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    sendHeader("Access-Control-Allow-Headers", "Content-Type");

    send(200, "application/json", getStatesAsJson());
  });

  /*
  GET method for fetching sensor addresses & current temps for assignment
  */
 on("/sensors", HTTP_GET, [this, &sensors]() {

    sendHeader("Access-Control-Allow-Origin", "*");
    sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    sendHeader("Access-Control-Allow-Headers", "Content-Type");

    std::vector<SensorData> sensorList = sensors.getSensorData();

    Serial.println(sensorList.size());

    JsonDocument  doc; // Adjust size if needed
    JsonArray sensorArray = doc.createNestedArray("sensors");

    for (const auto& sensor : sensorList) {
        JsonObject obj = sensorArray.createNestedObject();
        char addressStr[25];
        sprintf(addressStr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
            sensor.address[0], sensor.address[1], sensor.address[2], sensor.address[3], 
            sensor.address[4], sensor.address[5], sensor.address[6], sensor.address[7]);
        obj["address"] = addressStr;
        obj["temperature"] = sensor.temperature;
        Serial.println(addressStr);
        Serial.println(sensor.temperature);
    }

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    send(200, "application/json", jsonResponse);
 });

};

/*
Private method to stream files from SPIFFS memory. Streaming is crucial here, not to load whole file into RAM 
but instead sending it bit by bit. Type defaults to text/html but can be overwritten if needed when streaming css files etc.
*/
void WebServerManager::_streamFile(const char* path, const char* type) {
    File file = LittleFS.open(path, "r");
        if (!file) {
            send(404, "text/plain", "File not found");
            return;
        }

        // Set the Content-Type header
        sendHeader("Content-Type", type);

        size_t sent = streamFile(file, type);
    file.close();
};