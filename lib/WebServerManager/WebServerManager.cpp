#include <WebServerManager.h>

/**
 * @brief The constructor extends the ESP8266WebServer constructor, and takes multiple object references for interaction as parameter
 * 
 * Constructor takes intially the port for HTTP-requests as parameter, but in addition references to all objects
 * that need to be interacted through WebUI. ConfigManager object allows configs to be read & stored through UI,
 * SensorManager allows sensorvalues & addresses to be accessed and relay-object reference allows the relay to be controlled.
 * In addition, routes utilize few global functions declared in main program to avoid excess object references.
 * 
 * Constructor holds definitions of all routes involved in the program which are correspondingly commented in the code. The routes serve
 * following purposes
 * 
 * 1) GET routes to serve static files from SPIFFS (.html, .js, .css)
 * 2) GET routes to serve HTML-fragments from either SPIFFS or generated in JS file
 * 3) GET routes to return JSON objects for dynamic content update in WebUI (sensor values etc.)
 * 4) GET routes to control the device with query parameters (relay) and restart etc
 * 4) POST routes to receive configuration data as JSON in body and validate & store it through configmanager
 * 
 * The static files are sending via streaming to avoid loading files into RAM as whole and avoid memory issues. Currently CORS is enabled for each
 * route to ease debugging.
 * 
 * Routes return HTTP-codes alongside with messages to be alerted in browser as feedback
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
            send(200, "text/html", "Settings saved succesfully, after finalizing configuration restart device from status page or unplug!");

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

/**
 * @brief Private method to stream files
 * 
 * Streams the static files in chunks to avoid loading them into RAM and running out of memory.
 * Sends "text/plain" as default content type unless defined in parameter
 * 
 * @param path The filepath to open and stream in SPIFFS memory e.g. /index.html
 * @param type The Content-Type of response, e.g. "text/plain" or "application/json"
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