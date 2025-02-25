#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class Relay {

public:
    Relay(int relayPin);
    void on();
    void off();
    const char* getState(); // Return the current state of relay
private:
    int _relayPin;
    const char* _state = "OFF"; // Store the state of relay after toggling
    void setState(const char* state);
};

#endif