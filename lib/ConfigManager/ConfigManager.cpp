#include <ConfigManager.h>

// Constructor
ConfigManager::ConfigManager(FileManager &fm) : _fm(fm) {
}

/*
* Deserializes the config.json file into json document if exists, and places the config values in the instances member variables
* The values have been lightly validated when saving the config.json through UI so there should not need for validation when loading.
*/
void ConfigManager::loadConfig() {
    // Read config from file and deserialize into json doc
    String configString = _fm.readFile("/config.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configString);

    // Verify deserialization errors and find ssid and password ### Change this to copy the value instead of assigning into json doc
    if(error) {
        Serial.print("Failed to deserialize config JSON: ");
        Serial.println(error.f_str());
    } else {

        if (doc.containsKey("ssid")) { setSSID(doc["ssid"]); }
        if (doc.containsKey("password")) { setWiFiPassword(doc["password"]); }
        if (doc.containsKey("mqttServer")) { setMqttServer(doc["mqttServer"]); }
        if (doc.containsKey("mqttPort")) { setMqttPort(String(doc["mqttPort"]).c_str()); }
        if (doc.containsKey("mqttUser")) { setMqttUser(doc["mqttUser"]); }
        if (doc.containsKey("mqttPassword")) { setMqttPassword(doc["mqttPassword"]); }
        if (doc.containsKey("inletTempStateTopic")) { setInletTempStateTopic(doc["inletTempStateTopic"]); }
        if (doc.containsKey("outletTempStateTopic")) { setOutletTempStateTopic(doc["outletTempStateTopic"]); }
        if (doc.containsKey("relayStateTopic")) { setRelayStateTopic(doc["relayStateTopic"]); }
        if (doc.containsKey("relaySetTopic")) { setRelaySetTopic(doc["relaySetTopic"]); }

        Serial.println("Configs loaded from json: ");
        serializeJsonPretty(doc, Serial);
    }
}

/*
* Serialize configs into json document and save into file
*/
bool ConfigManager::saveConfig() {
    JsonDocument doc;

    doc["ssid"] = _SSID;
    doc["password"] = _wifiPassword;
    doc["mqttServer"] = _mqttServer.toString();
    doc["mqttPort"] = _mqttPort;
    doc["mqttUser"] = _mqttUser;
    doc["mqttPassword"] = _mqttPassword;
    doc["inletTempStateTopic"] = _inletTempStateTopic;
    doc["outletTempStateTopic"] = _outletTempStateTopic;
    doc["relayStateTopic"] = _relayStateTopic;
    doc["relaySetTopic"] = _relaySetTopic;

    String serializedConfig;
    serializeJson(doc, serializedConfig);

    if(!_fm.saveFile("/config.json", serializedConfig)) {
        return false;
    }    

    return true;
}

/*
Validate lenght of char input to make sure the input is not empty and it fits into memory allocated to member variable

PARAMS
    - const char* data, the string to validate
    - unsigned long targetSize, the sizeof target variable
RETURN
    - boolean
*/
bool ConfigManager::validateLength(const char* data, unsigned long targetSize) {
    if (strlen(data) == 0 || strlen(data) > targetSize) {
        return false;
    } 

    return true;
}

/*
Getters and setters

All member variables have getters and setters. String inputs are copied from into member variable
and ensured that they have null terminator for further use, and that they fit into target variable.
Setters return boolean value for the caller to know if it was succesful or not

PARAMS:
    - values to set into config, type dependant of configvariable
RETURN:
    - boolean
*/

const char* ConfigManager::getSSID() {
    return _SSID;
}

bool ConfigManager::setSSID(const char* ssid) {
    if(!validateLength(ssid, sizeof(_SSID))) {
        return false;
    };

    strncpy(_SSID, ssid, sizeof(_SSID) - 1); // Copy value up to length-1 from _ssid to have space for null terminator
    _SSID[sizeof(_SSID) - 1] = '\0'; // Ensure null-termination

    return true;
}

const char* ConfigManager::getWiFiPassword() {
    return _wifiPassword;
}

bool ConfigManager::setWiFiPassword(const char* password) {
    if(!validateLength(password, sizeof(_wifiPassword))) {
        return false;
    };

    strncpy(_wifiPassword, password, sizeof(_wifiPassword) - 1);
    _wifiPassword[sizeof(_wifiPassword) - 1] = '\0';

    return true;
}

IPAddress ConfigManager::getMqttServer() {
    return _mqttServer;
}

bool ConfigManager::setMqttServer(const char* mqttServer) {
    // Parse and validate IP address with method that returns false if string cannot be parsed to IPAddress object
    if(!_mqttServer.fromString(mqttServer)) {
        return false;
    };

    return true;
}

int ConfigManager::getMqttPort() {
    return _mqttPort;
}

bool ConfigManager::setMqttPort(const char* mqttPort) {
    // Parse and validate port. Atoi() returns 0 if value cannot be parsed to int, and 0 is invalid port value
    // Also check that given port is in proper range
    int port = atoi(mqttPort);
    if(port > 0 && port <= 65535) {
        _mqttPort = port;
        return true;
    }

    return false;
}

const char* ConfigManager::getMqttUser() {
    return _mqttUser;
}

bool ConfigManager::setMqttUser(const char* mqttUser) {
    if(!validateLength(mqttUser, sizeof(_mqttUser))) {
        return false;
    };

    strncpy(_mqttUser, mqttUser, sizeof(_mqttUser) - 1);
    _mqttUser[sizeof(_mqttUser) - 1] = '\0';

    return true;
}

const char* ConfigManager::getMqttPassword() {
    return _mqttPassword;
}

bool ConfigManager::setMqttPassword(const char* mqttPassword) {
    if(!validateLength(mqttPassword, sizeof(_mqttPassword))) {
        return false;
    };

    strncpy(_mqttPassword, mqttPassword, sizeof(_mqttPassword) - 1);
    _mqttPassword[sizeof(_mqttPassword) - 1] = '\0';

    return true;
}

const char* ConfigManager::getInletTempStateTopic() {
    return _inletTempStateTopic;
}

bool ConfigManager::setInletTempStateTopic(const char* inletStateTopic) {
    if(!validateLength(inletStateTopic, sizeof(_inletTempStateTopic))) {
        return false;
    };

    strncpy(_inletTempStateTopic, inletStateTopic, sizeof(_inletTempStateTopic) - 1);
    _inletTempStateTopic[sizeof(_inletTempStateTopic) - 1] = '\0';

    return true;
}

const char* ConfigManager::getOutletTempStateTopic() {
    return _outletTempStateTopic;
}

bool ConfigManager::setOutletTempStateTopic(const char* outletStateTopic) {
    if(!validateLength(outletStateTopic, sizeof(_outletTempStateTopic))) {
        return false;
    };

    strncpy(_outletTempStateTopic, outletStateTopic, sizeof(_outletTempStateTopic) - 1);
    _outletTempStateTopic[sizeof(_outletTempStateTopic) - 1] = '\0';

    return true;
}

const char* ConfigManager::getRelayStateTopic() {
    return _relayStateTopic;
}

bool ConfigManager::setRelayStateTopic(const char* relayStateTopic) {
    if(!validateLength(relayStateTopic, sizeof(_relayStateTopic))) {
        return false;
    };

    strncpy(_relayStateTopic, relayStateTopic, sizeof(_relayStateTopic) - 1);
    _relayStateTopic[sizeof(_relayStateTopic) - 1] = '\0';

    return true;
}

const char* ConfigManager::getRelaySetTopic() {
    return _relaySetTopic;
}

bool ConfigManager::setRelaySetTopic(const char* relaySetTopic) {
    if(!validateLength(relaySetTopic, sizeof(_relaySetTopic))) {
        return false;
    };

    strncpy(_relaySetTopic, relaySetTopic, sizeof(_relaySetTopic) - 1);
    _relaySetTopic[sizeof(_relaySetTopic) - 1] = '\0';
    return true;
}