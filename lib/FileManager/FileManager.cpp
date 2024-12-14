#include <FileManager.h>

FileManager::FileManager() {
}

bool FileManager::begin() {
    // Mount the filesystem to store and retrieve configurations
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount SPIFFS");
        return true;
    } else {
        Serial.println("SPIFFS mounted succesfully");
        return false;
    }
}

/*
Save data to path specified as parameter
Parameters:
    - const char* filename (path)
    - String data
Return:
    - bool, true on success and false on failure
*/
bool FileManager::saveFile(const char* filename, const String &data) {
    File file = LittleFS.open(filename, "w"); // Open file in write mode

    if(!file) {
        Serial.println("Opening file for saving failed!");
        return false;
    }

    if(!file.print(data)) {
        Serial.print("Saving file failed: ");
        Serial.println(filename);
        return false;
    } else {
        Serial.print("Saved file: ");
        Serial.println(filename);
    }

    file.close();
    return true;
}

/*
Reads file from specified path and returns content as string or empty string if failed to open file
Params:
    - const char* filename (path)
Return:
    - String contents if file found
    - String "" if failed to open file
*/
String FileManager::readFile(const char* filename) {
    File file = LittleFS.open(filename, "r"); // Open file in read mode

    String content = ""; // Prepare a string to chars from file

    if(!file) {
        Serial.println("Opening file for reading failed, file might not exist!");
        return "";
    }

    while(file.available()) {
        content += (char)file.read(); // Append the file contents to full string until end
    }

    file.close(); // Close the file after writing

    return content; // Return the file contents
}