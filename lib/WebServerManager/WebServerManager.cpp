#include <WebServerManager.h>
#include <html_index.h>
#include <json_utils.h>

// Constructor calls the base class constructor and takes the port and reference to filemanager as reference. Fm can then be used to store and retrieve config values.
WebServerManager::WebServerManager(uint16_t port, FileManager &fm) : ESP8266WebServer(port) {
    // Define routes

    // The root route. Need to capture 'this' to be able to access a method of parent class from lambda
    on("/", HTTP_GET, [this]() {
        send(200, "text/html", index_html); // Send the config page as response
    });

    // CORS DEBUGGING PURPOSES
    on("/savesettings", HTTP_OPTIONS, [this]() {
    // Handle preflight CORS request
    sendHeader("Access-Control-Allow-Origin", "*");
    sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    sendHeader("Access-Control-Allow-Headers", "Content-Type");
    send(204); // No Content
    });

    on("/savesettings", HTTP_POST, [this, &fm]() {
        // FOR DEBUGGING PURPOSES 
        sendHeader("Access-Control-Allow-Origin", "*");
        sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        sendHeader("Access-Control-Allow-Headers", "Content-Type");

        if(hasArg("plain")) {
            String bodyJsonString = arg("plain"); // Get the json string from request body

            JsonObject obj; // Prepare a JsonObject

            // Try to deserialize with utils function, if fails send statuscode 400
            if (!deserializeStringToJson(bodyJsonString, obj)) {
                send(400, "text/plain", "Invalid JSON");
                return;
            }

            Serial.println(bodyJsonString);

            fm.saveFile("/config.json", bodyJsonString);

            // Send OK response
            send(200, "text/html", "Settings saved succesfully");
        }


    });

}