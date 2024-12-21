#ifndef ConfigManager_h
#define ConfigManager_h

#include <FileManager.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <IPAddress.h>

class ConfigManager {
    public:
        ConfigManager(FileManager &fm);
        void loadConfig();
        bool saveConfig();

        // Getters
        const char* getSSID();
        const char* getWiFiPassword();
        IPAddress getMqttServer();
        int getMqttPort();
        const char* getMqttUser();
        const char* getMqttPassword();
        const char* getInletTempStateTopic();
        const char* getOutletTempStateTopic();
        const char* getRelayStateTopic();
        const char* getRelaySetTopic();

        // Setters
        bool setSSID(const char* ssid);
        bool setWiFiPassword(const char* password);
        bool setMqttServer(const char* mqttServer);
        bool setMqttPort(const char* mqttPort);
        bool setMqttUser(const char* mqttUser);
        bool setMqttPassword(const char* mqttPassword);
        bool setInletTempStateTopic(const char* inletStateTopic);
        bool setOutletTempStateTopic(const char* outletStateTopic);
        bool setRelayStateTopic(const char* relayStateTopic);
        bool setRelaySetTopic(const char* relaySetTopic);

    private:
        FileManager &_fm;

        // WiFi configs
        char _SSID[32] = "";
        char _wifiPassword[64] = "";

        // MQTT configs
        IPAddress _mqttServer = IPAddress(0, 0, 0, 0); // PubSubClient needs IPAddress object
        int _mqttPort = 0;
        char _mqttUser[32] = "";
        char _mqttPassword[64] = "";
        
        // MQTT topics
        char _inletTempStateTopic[64] = "";
        char _outletTempStateTopic[64] = "";
        char _relayStateTopic[64] = "";
        char _relaySetTopic[64] = "";

        // Validate lenght for strings in setters
        bool validateLength(const char* data, unsigned long target);
};

#endif