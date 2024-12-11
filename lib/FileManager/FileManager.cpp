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

// Save provided data into specified path
void FileManager::saveFile(const char* filename, const String &data) {
    File file = LittleFS.open(filename, "w"); // Open file in write mode

    if(!file) {
        Serial.println("Opening file for saving failed!");
    }

    if(!file.print(data)) {
        Serial.print("Saved file: ");
        Serial.println(filename);
    } else {
        Serial.print("Saving file failed: ");
        Serial.println(filename);
    }

    file.close();
}

// Reads the file contents from specified path and returns contents as string
String FileManager::readFile(const char* filename) {
    File file = LittleFS.open(filename, "r"); // Open file in read mode

    String content = ""; // Prepare a string to chars from file

    if(!file) {
        Serial.println("Opening file for saving failed!");
        return "";
    }

    while(file.available()) {
        content += (char)file.read(); // Append the file contents to full string until end
    }

    file.close(); // Close the file after writing

    return content; // Return the file contents
}