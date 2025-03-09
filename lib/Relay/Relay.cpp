#include "Relay.h"

/**
 * @brief Consturctor sets the relay object with I/O pin number where controllable relay is wired to
 * 
 * The constructor sets the requested pin number into private member variable for further reference,
 * and initially sets the I/O pinmode to output and OUTPUT_OPEN_DRAIN which is needed because the relay
 * needs to be drained to ground to be activated. When initializing the relaypin is set HIGH which
 * means the relay is inactive after initialization
 * 
 * @param relayPin The I/O pin number on ESP8266 device
 */
Relay::Relay (int relayPin) {
    _relayPin = relayPin;
    pinMode(_relayPin, OUTPUT_OPEN_DRAIN);
    digitalWrite(_relayPin, HIGH);
};

/**
 * @brief Sets the I/O pin state to LOW which activates the relay
 * 
 * Sets the relayPin LOW using digitalWrite, prints the state change and sets the 
 * relay object's internal state tracker to corresponding value.
 */
void Relay::on() {
    digitalWrite(_relayPin, LOW);
    Serial.println("Relay set ON");
    setState("ON");
};

/**
 * @brief Sets the I/O pin state to HIGH which deactivates the relay
 * 
 * Sets the relayPin HIGH using digitalWrite, prints the state change and sets the 
 * relay object's internal state tracker to corresponding value.
 */
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

/**
 * @brief Toggles the relaystate between ON/OFF based on persisted previous state
 */
void Relay::toggle() {
    if (_state == "ON") {
        digitalWrite(_relayPin, HIGH);
        setState("OFF");
    } else {
        digitalWrite(_relayPin, LOW);
        setState("ON");
    }
};