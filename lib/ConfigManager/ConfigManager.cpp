#include <ConfigManager.h>

/**
 * @brief Constructor creates a ConfigManager object utilizing a FileManager object to interact with FS
 * 
 * FileManager object is passed to constructor as a reference.
 */
ConfigManager::ConfigManager(FileManager &fm) : _fm(fm) {
}

/**
 * @brief Reads existing configuration from /config.json file and sets them as active config through setters
 * 
 * Method utilizes FileManager object to access config.json file inthe SPIFFS memory and read the configs.
 * Activating config through setter methods validates the configs on the fly.
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
        if (doc.containsKey("inletSensorAddress")) { setInletSensorAddress(doc["inletSensorAddress"]); }
        if (doc.containsKey("outletSensorAddress")) { setOutletSensorAddress(doc["outletSensorAddress"]); }

        Serial.println("Configs loaded from json: ");
        serializeJsonPretty(doc, Serial);
    }
}

/**
 * @brief Saves existing member variables of ConfigManager object to SPIFFS memory as persisted config.json
 * 
 * Method composes a JSON-string from all member variables of config manager object and stores it into
 * config.json file with filemanager. File is always overwritten.
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
    doc["inletSensorAddress"] = _inletSensorAddress;
    doc["outletSensorAddress"] = _outletSensorAddress;

    String serializedConfig;
    serializeJson(doc, serializedConfig);

    if(!_fm.saveFile("/config.json", serializedConfig)) {
        return false;
    }    

    return true;
}

/**
 * @brief Returns all configs excluding passwords as JSON-string
 * 
 * Configs are needed in the WebUI for pre-populating forms etc., and there configs as json are handy.
 * Method composes JSON string but excludes sensitive passwords to be utilized elsewhere.
 * 
 * @return JSON-string containing all configurations except passwords
 */
String ConfigManager::getConfigAsJson() {
    JsonDocument doc;

    // Populate JSON document with current config values
    doc["ssid"] = _SSID;
    // doc["password"] = _wifiPassword; - Do not pass back WiFi password
    doc["mqttServer"] = _mqttServer.toString();
    doc["mqttPort"] = _mqttPort;
    doc["mqttUser"] = _mqttUser;
    // doc["mqttPassword"] = _mqttPassword; - Do not pass back Mqtt password
    doc["inletTempStateTopic"] = _inletTempStateTopic;
    doc["outletTempStateTopic"] = _outletTempStateTopic;
    doc["relayStateTopic"] = _relayStateTopic;
    doc["relaySetTopic"] = _relaySetTopic;

    // Serialize JSON document into a string
    String serializedConfig;
    serializeJson(doc, serializedConfig);

    return serializedConfig;
}

/**
 * @brief Validates the chars length to make sure it fits in the reserved space
 * 
 * Method receives the size of the target member variable where data will be stored, and current data
 * that will be stored and validates that memory allocation is not exceeded
 * 
 * @param data The chars to be stored into config manager
 * @param targetSize The space allocation of inteded member variable
 * @return true if data fits to target, else flase.
 */
bool ConfigManager::_validateLength(const char* data, unsigned long targetSize) {
    if (strlen(data) == 0 || strlen(data) > targetSize) {
        return false;
    } 

    return true;
}

const char* ConfigManager::getSSID() {
    return _SSID;
}

/**
 * @brief Stores SSID into ConfigManager member variable with validation
 * 
 * Data is validated to fit into variable, and then value is copied into
 * member variable and null-terminator is being added into variable.
 * Same logic applies to all setters.
 * 
 * @param ssid The SSID to be stored into member variable
 */
bool ConfigManager::setSSID(const char* ssid) {
    if(!_validateLength(ssid, sizeof(_SSID))) {
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
    if(!_validateLength(password, sizeof(_wifiPassword))) {
        return false;
    };

    strncpy(_wifiPassword, password, sizeof(_wifiPassword) - 1);
    _wifiPassword[sizeof(_wifiPassword) - 1] = '\0';

    return true;
}

IPAddress ConfigManager::getMqttServer() {
    return _mqttServer;
}

/**
 * @brief Validates and stores MQTT broker address
 * 
 * _mqttServer is type IPAddress, which contains method to parse IP-address from
 * string and validate it is proper. The received chars are parsed to IP and result is returned.
 * If IP address structure was it will fail.
 * 
 * @param mqttServer The MQTT broker address as chars
 * @return true if possible to parse into IPAddress, else false.
 */
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

/**
 * @brief Stores MQTT port as integer to member variable
 * 
 * Converts the received mqttPort as chars into integer, and lightly validates that the port is > 0 and < 65535
 * 
 * @param mqttPort as chars
 * @return true if was convertable and > 0 and < 65535 else false
 */
bool ConfigManager::setMqttPort(const char* mqttPort) {

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
    if(!_validateLength(mqttUser, sizeof(_mqttUser))) {
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
    if(!_validateLength(mqttPassword, sizeof(_mqttPassword))) {
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
    if(!_validateLength(inletStateTopic, sizeof(_inletTempStateTopic))) {
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
    if(!_validateLength(outletStateTopic, sizeof(_outletTempStateTopic))) {
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
    if(!_validateLength(relayStateTopic, sizeof(_relayStateTopic))) {
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
    if(!_validateLength(relaySetTopic, sizeof(_relaySetTopic))) {
        return false;
    };

    strncpy(_relaySetTopic, relaySetTopic, sizeof(_relaySetTopic) - 1);
    _relaySetTopic[sizeof(_relaySetTopic) - 1] = '\0';
    return true;
}

const char* ConfigManager::getInletSensorAddress() {
    return _inletSensorAddress;
}

bool ConfigManager::setInletSensorAddress(const char* inletSensorAddress) {
    if(!_validateLength(inletSensorAddress, sizeof(_inletSensorAddress))) {
        return false;
    };

    strncpy(_inletSensorAddress, inletSensorAddress, sizeof(_inletSensorAddress) - 1);
    _inletSensorAddress[sizeof(_inletSensorAddress) - 1] = '\0';
    return true;
}

const char* ConfigManager::getOutletSensorAddress() {
    return _outletSensorAddress;
}

bool ConfigManager::setOutletSensorAddress(const char* outletSensorAddress) {
    if(!_validateLength(outletSensorAddress, sizeof(_outletSensorAddress))) {
        return false;
    };

    strncpy(_outletSensorAddress, outletSensorAddress, sizeof(_outletSensorAddress) - 1);
    _outletSensorAddress[sizeof(_outletSensorAddress) - 1] = '\0';
    return true;
}