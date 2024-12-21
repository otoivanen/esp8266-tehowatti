#include "SensorManager.h"

const int SENSOR_READ_INTERVAL = 1000; // Sensor read interval in mSec

SensorManager::SensorManager(const int onewire_pin) : _oneWire(onewire_pin), _sensors(&_oneWire) {
    _sensors.begin();
}

void SensorManager::readSensors() {

    if (millis() - _lastRead > SENSOR_READ_INTERVAL){
        _sensors.requestTemperatures();
        _lastInletTemp = _inletTemp;
        _inletTemp = _sensors.getTempCByIndex(0);

        _lastOutletTemp = _outletTemp;
        _outletTemp = _sensors.getTempCByIndex(0);

        _lastRead = millis();

        Serial.print("Inlet temperature: ");
        Serial.println(String(_inletTemp).c_str());
        Serial.print("Outlet temperature: ");
        Serial.println(String(_outletTemp).c_str());
    }
}

void SensorManager::searchSensors() {

}

// Getters and setters
float SensorManager::getInletTemp() {
    return _lastInletTemp;
}

float SensorManager::getOutletTemp() {
    return _lastOutletTemp;
}

float SensorManager::getLastInletTemp() {
    return _lastInletTemp;
}

float SensorManager::getLastOutletTemp() {
    return _lastOutletTemp;
}