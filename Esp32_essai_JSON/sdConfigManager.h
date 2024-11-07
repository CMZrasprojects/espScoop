#ifndef SDCONFIGMANAGER_H
#define SDCONFIGMANAGER_H

#include <ArduinoJson.h>
#include <SD.h>

class sdConfigManager {
public:
    bool load(const char* fileName);
    bool save(const char* fileName);
    bool resetToDefault(const char* defaultFileName, const char* configFileName);
};

#endif
