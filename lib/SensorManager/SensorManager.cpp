#include "SensorManager.h"

const int SENSOR_READ_INTERVAL = 1000; // Sensor read interval in mSec

SensorManager::SensorManager(const int onewire_pin) : _oneWire(onewire_pin), _sensors(&_oneWire) {
    _sensors.begin();
};

void SensorManager::readSensors() {

    static unsigned long lastSensorRead = 0;

    if (millis() - lastSensorRead > SENSOR_READ_INTERVAL){
        _sensors.requestTemperatures();
        _lastInletTemp = _inletTemp;
        _inletTemp = _sensors.getTempC(_inletSensorAddress);

        _lastOutletTemp = _outletTemp;
        _outletTemp = _sensors.getTempC(_outletSensorAddress);

        lastSensorRead = millis();

    }
};

/*
Get sensor address and current temperatures as struct to fetch through WebUI
for assigning sensor addresses into configs
*/
std::vector<SensorData> SensorManager::getSensorData() {
    std::vector<SensorData> sensorList;
    // There is apparently a bug in the DS library, that requires .begin(); to be called
    // Twice to enter discovery mode correctly and provide the sensorcount
    _sensors.begin();
    _sensors.begin();
    sensorList.reserve(_sensors.getDeviceCount());

    _sensors.requestTemperatures();  // Get all sensor readings

    for (int i = 0; i < _sensors.getDeviceCount(); i++) {
        SensorData data;
        DeviceAddress address;

        if (_sensors.getAddress(address, i)) {
            for (int j = 0; j < 8; j++) {
                data.address[j] = address[j];
            }
            data.temperature = _sensors.getTempC(address);
            sensorList.push_back(data);
        }
    }

    return sensorList;  // Return address-temperature pairs
};

// Getters and setters
float SensorManager::getInletTemp() {
    return _inletTemp;
};

float SensorManager::getOutletTemp() {
    return _outletTemp;
};

float SensorManager::getLastInletTemp() {
    return _lastInletTemp;
};

float SensorManager::getLastOutletTemp() {
    return _lastOutletTemp;
};

const char* SensorManager::getInletSensorAddress() {

    static char addressStr[24];  // Buffer to store formatted address (23 chars + null terminator)

    snprintf(addressStr, sizeof(addressStr), "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
             _inletSensorAddress[0], _inletSensorAddress[1], _inletSensorAddress[2], _inletSensorAddress[3], 
             _inletSensorAddress[4], _inletSensorAddress[5], _inletSensorAddress[6], _inletSensorAddress[7]);

    return addressStr;  // Return the formatted string
};

void SensorManager::setInletSensorAddress(const char* inletSensorAddress) {
    _parseAddressFromString(inletSensorAddress, _inletSensorAddress);
};

const char* SensorManager::getOutletSensorAddress() {
    static char addressStr[24];  // Buffer to store formatted address (23 chars + null terminator)

    snprintf(addressStr, sizeof(addressStr), "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
             _outletSensorAddress[0], _outletSensorAddress[1], _outletSensorAddress[2], _outletSensorAddress[3], 
             _outletSensorAddress[4], _outletSensorAddress[5], _outletSensorAddress[6], _outletSensorAddress[7]);

    return addressStr;  // Return the formatted string
}

void SensorManager::setOutletSensorAddress(const char* outletSensorAddress) {
    _parseAddressFromString(outletSensorAddress, _outletSensorAddress);
};

void SensorManager::_parseAddressFromString(const char* addr, DeviceAddress &outAddress) {
    Serial.print("Parsing address: '");
    Serial.print(addr);
    Serial.println("'");  // 🔹 Check if unexpected characters are present

    int values[8];  // Storage for parsed values

    if (sscanf(addr, "%x:%x:%x:%x:%x:%x:%x:%x",
               &values[0], &values[1], &values[2], &values[3], 
               &values[4], &values[5], &values[6], &values[7]) == 8) {
        
        for (int i = 0; i < 8; i++) {
            outAddress[i] = static_cast<uint8_t>(values[i]);
        }

        //Print the parsed address
        Serial.print("Parsed address: ");
        for (int i = 0; i < 8; i++) {
            Serial.printf("%02X ", outAddress[i]);
        }
        Serial.println();
    } else {
        Serial.println("Invalid address format while parsing Sensor DeviceAddress!");
    }
};