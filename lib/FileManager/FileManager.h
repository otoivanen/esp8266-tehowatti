#ifndef FileManager_h
#define FileManager_h

#include <LittleFS.h>

// Create a class for using the LittleFS library to interact with files. It is not a class so we dont inherit
class FileManager {

public:
    FileManager();
    bool begin();
    bool saveFile(const char* filename, const String &data); // Save the data to file
    String readFile(const char* filename); // Read file and return String
};

#endif