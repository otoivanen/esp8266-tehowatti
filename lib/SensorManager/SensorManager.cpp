#include "SensorManager.h"

const int SENSOR_READ_INTERVAL = 1000; // Sensor read interval in mSec

/**
 * @brief Constructor takes the onewire I/O-pin as parameter and creates assigned onewire &sensors objects
 * 
 * OneWire pin is needed to initialize the OneWire communication with sensors. _sensors is DallasTemperatureSensor type
 * object member variable that utilizes OneWire bus, initialized while the Sensor Manager is initialized.
 * Constructor also starts the sensor communication.
 */
SensorManager::SensorManager(const int onewire_pin) : _oneWire(onewire_pin), _sensors(&_oneWire) {
    _sensors.begin();
};

/**
 * @brief Core method to read the sensor values on predefined interval from configured sensor addresses
 * 
 * The method tracks the previous reading timestamp (ms) with static variable. Sensor readings are 
 * fetched when defined SENSOR_READ_INTERVAL has passed in non-blocking manner. Reading sensors 
 * is time consuming operation so it is done in timeintervals to allow HTTP- and MQTT- and other operations 
 * to be handled in between.
 * 
 * Sensors are being read based on the sensor addresses found from configuration file and stored into object's 
 * member variables. If sensor is not found or address is not configured, will set -127c as defaul temp.
 */
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

/**
 * @brief Generates array of SensorData structs to be returned to caller. Needed for WebUI configuration view.
 * 
 * Method re-initiates the sensors to be in discoverymode, and then bus is scanned to get number of connected sensors.
 * After scan, temperatures are requested and for each sensor found in count operation it's byte address and current
 * temperature are pushed to array to be returned. From addres-temp pair user can identify the sensors and set the config
 * 
 * @return ByteAddress-CurrentTemperature pairs as SensorData structs inside an array containing all sensors in the bus
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

/**
 * @brief Provides the sensor address (DeviceAddress) from member variable as const char*
 * 
 * The sensor address is stored as DeviceAddress object into member variable. It needs to be parsed into
 * chars from bytes before returning as char*
 */
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

/**
 * @brief Provides the sensor address (DeviceAddress) from member variable as const char*
 * 
 * The sensor address is stored as DeviceAddress object into member variable. It needs to be parsed into
 * chars from bytes before returning as char*
 */
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

/**
 * @brief Parses chars into DeviceAddress for storing sensor address into member variable
 * 
 * The sensor addresses are received from config manager and WebUI as plain text. They need to be
 * transformed into DeviceAddress type before assigning into member variable, and this method does the parsing
 * 
 * @param addr The address chars to be parsed
 * @param outAddress The DeviceAddress object initialized in the caller, the target for parsed value
 */
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