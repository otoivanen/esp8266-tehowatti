#include "json_utils.h"

// Serialize a JSON object to a string. References to original object instead of copying
String serializeJsonToString(const JsonObject &jsonObject) {
    String jsonString;
    JsonDocument doc;
    doc = jsonObject;
    serializeJson(doc, jsonString);
    return jsonString;
}

// Deserialize a JSON string to a JsonObject. This references to the original object so it modifies it.
bool deserializeStringToJson(const String& jsonString, JsonObject &jsonObject) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.print("Failed to deserialize JSON: ");
        Serial.println(error.f_str());
        return false;
    }

    jsonObject = doc.as<JsonObject>();
    return true;
}