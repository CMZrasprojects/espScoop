#include "sdConfigManager.h"
#include "configVariables.h"

// Définition de la liste des réseaux Wi-Fi
extern std::vector<WifiCredentials> wifiList;  // Liste vide initialement

bool sdConfigManager::load(const char* fileName) {
    File file = SD.open(fileName);
    if (!file) {
      Serial.printf("erreur de lecture du fichier");
      return false;
    }

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Erreur JSON : ");
        Serial.println(error.c_str());
        file.close();
        return false;
    }

    // Variables globales Network
    deviceName = doc["system"]["deviceName"] | "DefaultDevice";
    ethernetStatus = doc["network"]["interface"]["ethernet"]["status"] | 1;
    ethernetDhcp = doc["network"]["interface"]["ethernet"]["dhcp"] | 1;
    ethernetIpv4 = doc["network"]["interface"]["ethernet"]["address"]["ipv4"] | "192.168.1.51";
    ethernetMasque = doc["network"]["interface"]["ethernet"]["address"]["masque"] | "255.255.255.0";
    ethernetPasserelle = doc["network"]["interface"]["ethernet"]["address"]["passerelle"] | "192.168.1.1";
    wifiStatus = doc["network"]["interface"]["wifi"]["status"] | 1;
    wifiDhcp = doc["network"]["interface"]["wifi"]["dhcp"] | 1;
    wifiIpv4 = doc["network"]["interface"]["wifi"]["address"]["ipv4"] | "172.20.10.2";
    wifiMasque = doc["network"]["interface"]["wifi"]["address"]["masque"] | "255.255.255.0";
    wifiPasserelle = doc["network"]["interface"]["wifi"]["address"]["passerelle"] | "171.20.10.1";
    lteStatus = doc["network"]["interface"]["lte"]["status"] | 0;

    // Variables globales Audio
    headphoneLevel = doc["audio"]["headphoneLevel"] | 0.5;
    samplingRate = doc["audio"]["samplingRate"] | 48000;

    // Charger la liste Wi-Fi
    Serial.println("Chargement de la liste des réseaux Wi-Fi...");
    wifiList.clear();  // Vider la liste actuelle
    JsonArray wifiArray = doc["network"]["interface"]["wifi"]["wifiList"].as<JsonArray>();
    if (wifiArray.isNull()) {
        Serial.println("Aucune liste Wi-Fi trouvée dans le fichier de configuration.");
    } else {
        for (JsonObject wifi : wifiArray) {
            WifiCredentials creds;
            creds.ssid = wifi["ssid"].as<const char*>();
            creds.password = wifi["password"].as<const char*>();
            wifiList.push_back(creds);

            // Affichage des informations Wi-Fi pour chaque entrée ajoutée
            Serial.print("SSID : ");
            Serial.println(creds.ssid);
            Serial.print("Mot de passe : ");
            Serial.println(creds.password);
        }
    }
    Serial.println("Chargement de la configuration effectuée");
    return true;
}

bool sdConfigManager::save(const char* fileName) {
    StaticJsonDocument<4096> doc;
    doc["system"]["deviceName"] = deviceName;
    doc["network"]["interface"]["ethernet"]["status"] = ethernetStatus;
    doc["network"]["interface"]["ethernet"]["dhcp"] = ethernetDhcp;
    doc["network"]["interface"]["ethernet"]["address"]["ipv4"] = ethernetIpv4;
    doc["network"]["interface"]["ethernet"]["address"]["masque"] = ethernetMasque;
    doc["network"]["interface"]["ethernet"]["address"]["passerelle"] = ethernetPasserelle;
    doc["network"]["interface"]["wifi"]["status"] = wifiStatus;
    doc["network"]["interface"]["wifi"]["dhcp"] = wifiDhcp;
    doc["network"]["interface"]["wifi"]["address"]["ipv4"] = wifiIpv4;
    doc["network"]["interface"]["wifi"]["address"]["masque"] = wifiMasque;
    doc["network"]["interface"]["wifi"]["address"]["passerelle"] = wifiPasserelle;
    doc["network"]["interface"]["lte"]["status"] = lteStatus;

    doc["audio"]["headphoneLevel"] = headphoneLevel;
    doc["audio"]["samplingRate"] = samplingRate;

    // Sauvegarde de la liste wifiList
    JsonArray wifiArray = doc["network"]["interface"]["wifi"].createNestedArray("wifiList");
    for (const WifiCredentials& creds : wifiList) {
        JsonObject wifiObj = wifiArray.createNestedObject();
        wifiObj["ssid"] = creds.ssid;
        wifiObj["password"] = creds.password;
    }

    File file = SD.open(fileName, FILE_WRITE);
    if (!file) {
      Serial.printf("erreur d'ouverture du fichier pour écriture");
     return false;
    }
    bool success = serializeJson(doc, file) > 0;
    file.close();
    return success;
}

bool sdConfigManager::resetToDefault(const char* defaultFileName, const char* configFileName) {
    File defaultFile = SD.open(defaultFileName);
    if (!defaultFile) return false;

    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, defaultFile)) return false;
    defaultFile.close();

    File configFile = SD.open(configFileName, FILE_WRITE);
    if (!configFile) return false;

    bool success = serializeJson(doc, configFile) > 0;
    configFile.close();
    return success && load(configFileName);
}
