#include <WebServerManager.h>

// Constructor calls the base class constructor and takes the port as parameter
WebServerManager::WebServerManager(uint16_t port) : ESP8266WebServer(port) {

    // Define routes

    // The root route. Need to capture 'this' to be able to access a method of parent class from lambda
    on("/", HTTP_GET, [this]() {
        // Send a response when the root path is accessed
        send(200, "text/html", "<h1>Tämä on testi</h1>");
    });

}