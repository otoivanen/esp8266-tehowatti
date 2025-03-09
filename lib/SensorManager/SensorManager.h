/*
SensorManager class contains DallasTemperature sensor object as attribute which allows interacting with
DS18B20 sensors in I2C bus. The class provides additional methods for interacting and storing state-
and address information and distribute it across the program

Author: Oskari Toivanen
Date: 2025-03-09
*/

#ifndef SensorManager_h
#define SensorManager_h

#include <DallasTemperature.h>
#include <OneWire.h>
#include <vector>

// A struct for holding sensor address + current temp values 
struct SensorData {
    uint8_t address[8];  // Store sensor address (8 bytes)
    float temperature; // Store current temperature
};

class SensorManager {
public:
    SensorManager(const int onewire_pin);
    void readSensors(); // Update sensorvalues
    std::vector<SensorData> getSensorData(); // Search available sensor addresses
    
    float getInletTemp();
    float getOutletTemp();
    float getLastInletTemp();
    float getLastOutletTemp();
    
    const char* getInletSensorAddress();
    void setInletSensorAddress(const char* inletSensorAddress);
    const char* getOutletSensorAddress();
    void setOutletSensorAddress(const char* outletSensorAddress);

private:
    OneWire _oneWire;
    DallasTemperature _sensors;

    // Store the correct sensor addresses
    DeviceAddress _inletSensorAddress;
    DeviceAddress _outletSensorAddress;

    // Current measured temperatures
    float _inletTemp = 0.00;
    float _outletTemp = 0.00;

    // Last measured temperatures
    float _lastInletTemp = 0.00;
    float _lastOutletTemp = 0.00;

    // Utils
    void _parseAddressFromString(const char* addr, DeviceAddress &outAddress);
};

#endif
