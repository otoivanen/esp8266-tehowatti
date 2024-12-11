/*
Helper utility to serialize and deserialize between String and JSON
*/

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <ArduinoJson.h>

// Function to serialize a JSON object to a string. Takes reference to source jsonObject as parameter
String serializeJsonToString(const JsonObject &jsonObject);

/*
Deserializes raw JSON string into json object by modifying the original object. Returns true if
succesful, false if JSON was invalid
*/
bool deserializeStringToJson(const String &jsonString, JsonObject &jsonObject);

#endif