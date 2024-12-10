#include <Arduino.h>
#include <WiFiManager.h>
#include <secrets.h>

WiFiManager wm;

void setup() {

  // Open the serial port
  Serial.begin(9600);

  // Connect to WiFi with ssid and credentials
  wm.Connect(ssid, pw);
}

void loop() {

}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}