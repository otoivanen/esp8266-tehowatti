#include <FileManager.h>

FileManager::FileManager() {
}

/**
 * @brief Tries to mount filesystem into use
 * 
 * @return True if succesfully mounted, else false
 */
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

/**
 * @brief Saves data into file in SPIFFS
 * 
 * Method creates the file given as parameter and writes the data, doing needed validity checks
 * File is being properly closed after writing.
 * 
 * @param filename The filename to be stored into SPIFFS
 * @param data The data contents to be written into file
 * 
 * @return True if saved succesfully, else false
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

/**
 * @brief Read file with given filename and return the contents as string
 * 
 * Opens the file defined as parameter, and appends the content chars as long as they are
 * available to result string. After reading the file is properly closed.A0
 * 
 * @param filename The filename to be read
 * 
 * @return Filecontents as String
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