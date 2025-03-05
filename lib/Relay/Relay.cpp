#include "Relay.h"

// Constructor
Relay::Relay (int relayPin) {
    _relayPin = relayPin;
    pinMode(_relayPin, OUTPUT_OPEN_DRAIN);
    digitalWrite(_relayPin, HIGH);
};

void Relay::on() {
    digitalWrite(_relayPin, LOW);
    Serial.println("Relay set ON");
    setState("ON");
};

void Relay::off() {
    digitalWrite(_relayPin, HIGH);
    Serial.println("Relay set OFF");
    setState("OFF");
};

void Relay::setState(const char* state) {
    _state = state;
};

const char* Relay::getState() {
    return _state;
};

void Relay::toggle() {
    if (_state == "ON") {
        digitalWrite(_relayPin, HIGH);
        setState("OFF");
    } else {
        digitalWrite(_relayPin, LOW);
        setState("ON");
    }
};