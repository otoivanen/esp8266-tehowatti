/*
Relay class encapsulates the functionalities needed for controlling relay through device IO, and keeps track of the
led's current state as descriptive text.

Author: Oskari Toivanen
Date: 2025-03-09
*/

#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class Relay {

public:
    Relay(int relayPin);
    void on();
    void off();
    void toggle();
    const char* getState(); // Return the current state of relay
private:
    int _relayPin;
    const char* _state = "OFF"; // Store the state of relay after toggling
    void setState(const char* state);
};

#endif