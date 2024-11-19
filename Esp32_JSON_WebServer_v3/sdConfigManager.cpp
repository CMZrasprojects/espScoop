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

    Serial.println("Chargement des variables globales");

    // Variables globales Network
    deviceName = doc["system"]["deviceName"] | "DefaultDevice";
    ethernetStatus = doc["network"]["interface"]["ethernet"]["status"] | 1;
    ethernetDhcp = doc["network"]["interface"]["ethernet"]["dhcp"] | 1;
    //
    String ethernetIpv4FromSd = doc["network"]["interface"]["ethernet"]["address"]["ipv4"] | "192.168.1.51";
    if (ethernetIpv4.fromString(ethernetIpv4FromSd)) {
        Serial.print("Adresse IP chargée avec succès : ");
        Serial.println(ethernetIpv4);
    } else {
        Serial.println("Erreur : Adresse IP invalide dans le JSON, utilisation de l'adresse par défaut");
        ethernetIpv4.fromString("192.168.1.51"); // Valeur par défaut
    }
    String ethernetMasqueFromSd = doc["network"]["interface"]["ethernet"]["address"]["masque"] | "255.255.255.0";
    if (ethernetMasque.fromString(ethernetMasqueFromSd)) {
        Serial.print("Adresse IP chargée avec succès : ");
        Serial.println(ethernetMasque);
    } else {
        Serial.println("Erreur : Adresse IP invalide dans le JSON, utilisation de l'adresse par défaut");
        ethernetMasque.fromString("255.255.255.0"); // Valeur par défaut
    }
    String ethernetPasserelleFromSd = doc["network"]["interface"]["ethernet"]["address"]["passerelle"] | "192.168.1.1";
    if (ethernetPasserelle.fromString(ethernetPasserelleFromSd)) {
        Serial.print("Adresse IP chargée avec succès : ");
        Serial.println(ethernetPasserelle);
    } else {
        Serial.println("Erreur : Adresse IP invalide dans le JSON, utilisation de l'adresse par défaut");
        ethernetMasque.fromString("192.168.1.1"); // Valeur par défaut
    }
    wifiStatus = doc["network"]["interface"]["wifi"]["status"] | 1;
    wifiDhcp = doc["network"]["interface"]["wifi"]["dhcp"] | 1;
    String wifiIpv4FromSd = doc["network"]["interface"]["wifi"]["address"]["ipv4"] | "172.20.10.2";
    if (wifiIpv4.fromString(wifiIpv4FromSd)) {
        Serial.print("Adresse IP chargée avec succès : ");
        Serial.println(wifiIpv4);
    } else {
        Serial.println("Erreur : Adresse IP invalide dans le JSON, utilisation de l'adresse par défaut");
        wifiIpv4.fromString("172.20.10.2"); // Valeur par défaut
    }
    String wifiMasqueFromSd = doc["network"]["interface"]["wifi"]["address"]["masque"] | "255.255.255.0";
    if (wifiMasque.fromString(wifiMasqueFromSd)) {
        Serial.print("Adresse IP chargée avec succès : ");
        Serial.println(wifiMasque);
    } else {
        Serial.println("Erreur : Adresse IP invalide dans le JSON, utilisation de l'adresse par défaut");
        wifiMasque.fromString("255.255.255.0"); // Valeur par défaut
    }
    String wifiPasserelleFromSd = doc["network"]["interface"]["wifi"]["address"]["passerelle"] | "171.20.10.1";
    if (wifiPasserelle.fromString(wifiPasserelleFromSd)) {
        Serial.print("Adresse IP chargée avec succès : ");
        Serial.println(wifiPasserelle);
    } else {
        Serial.println("Erreur : Adresse IP invalide dans le JSON, utilisation de l'adresse par défaut");
        wifiPasserelle.fromString("172.20.10.1"); // Valeur par défaut
    }
    lteStatus = doc["network"]["interface"]["lte"]["status"] | 0;

    // Variables globales Audio
    headphoneLevel = doc["audio"]["headphoneLevel"] | 0.5;
    sampleRate = doc["audio"]["sampleRate"] | 48000;

    Serial.println("Chargement des variables OK");

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

            Serial.println("Chargement de la liste Wifi OK");
        }
    }
    Serial.println("Chargement de la configuration effectuée");
    return true;
}

bool sdConfigManager::save(const char* fileName) {
    Serial.println("Sauvegarde des variables globales mises à jour...");
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
    doc["audio"]["sampleRate"] = sampleRate;

    Serial.println("Sauvegarde de la liste Wifi mise à jour...");
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
    Serial.println("Sauvegarde effectuée.");
    return success;
}

bool sdConfigManager::resetToDefault(const char* defaultFileName, const char* configFileName) {
    File defaultFile = SD.open(defaultFileName);
    if (!defaultFile) {
      Serial.println("Reset Usine : pas de fichier usine");
      return false;
    }
    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, defaultFile)) {
      Serial.println("Reset Usine : erreur de deserialisation JSON");
      return false;
    }
    defaultFile.close();

    File configFile = SD.open(configFileName, FILE_WRITE);
    if (!configFile) {
      Serial.println("Reset Usine : pas de fichier config sur lequel écrire");
      return false;
    }

    bool success = serializeJson(doc, configFile) > 0;
    configFile.close();
    Serial.println("Reset Usine effectué");
    return success && load(configFileName);
}
