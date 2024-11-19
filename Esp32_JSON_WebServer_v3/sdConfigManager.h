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

// Structure pour stocker les informations de chaque réseau Wi-Fi
struct WifiCredentials {
    const char* ssid;
    const char* password;
};

// Déclaration de la liste des réseaux Wi-Fi
extern std::vector<WifiCredentials> wifiList;

#endif
