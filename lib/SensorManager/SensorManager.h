#ifndef SensorManager_h
#define SensorManager_h

#include <DallasTemperature.h>
#include <OneWire.h>

class SensorManager {
public:
    SensorManager(const int onewire_pin);
    void readSensors(); // Update sensorvalues
    void searchSensors(); // Search available sensor addresses
    
    float getInletTemp();
    float getOutletTemp();
    float getLastInletTemp();
    float getLastOutletTemp();

private:
    OneWire _oneWire;
    DallasTemperature _sensors;

    unsigned long _lastRead = millis(); // Last reading event

    // Store the correct sensor addresses
    DeviceAddress _inletSensorAddress;
    DeviceAddress _outletSensorAddress;

    // Current measured temperatures
    float _inletTemp = 0.00;
    float _outletTemp = 0.00;

    // Last measured temperatures
    float _lastInletTemp = 0.00;
    float _lastOutletTemp = 0.00;
};

#endif
