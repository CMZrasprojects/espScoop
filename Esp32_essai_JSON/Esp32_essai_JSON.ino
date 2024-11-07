/**
 * @file      
 * @author    C.Moizeau
 * @license   
 * @copyright 
 * @date      
 *
 */
#include <Arduino.h>
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <ETHClass2.h>       //Is to use the modified ETHClass
#define ETH  ETH2
#else
#include <ETH.h>
#endif
#include <SPI.h>
#include <SD.h>
#include "utilities.h"          //Board PinMap
#include <WiFi.h>
#include <WebServer.h>
#include <husarnet.h>
#include <ArduinoJson.h>
#include <sdConfigManager.h>

/* Définition des Variables globales :
*******************************************************************
*/
const int factoryResetPin = 46; // Pin du bouton pour le reset usine
const unsigned long resetDelay = 5000;  // Durée (en ms) d'appui pour déclencher le reset
bool factoryResetTriggered = false;

// Variables globales Network
const char* deviceName = "name";
bool ethernetStatus = 0;
bool ethernetDhcp = 0;
const char* ethernetIpv4 = "0.0.0.0";
const char* ethernetMasque = "0.0.0.0";
const char* ethernetPasserelle = "0.0.0.0";
bool wifiStatus = 0;
bool wifiDhcp = 0;
const char* wifiIpv4 = "0.0.0.0";
const char* wifiMasque = "0.0.0.0";
const char* wifiPasserelle = "0.0.0.0";
bool lteStatus = 0;

// Variables globales Audio
float headphoneLevel = 0;
int samplingRate = 0;


/* Définition des "Class" utilisées :
*******************************************************************
*/

sdConfigManager config;


/* Fonctions
*******************************************************************
*/





/* Fonction Setup
*******************************************************************
*/

void setup() {

  Serial.begin(115200);

  initSD();
  Serial.println("Carte SD initialisée");

  Serial.println("Chargement de la configuration sauvegardée...");
  config.load("/config.json");

}
/* Fonction Loop
*******************************************************************
*/

void loop() {


}

/* Fonctions de base : initialisation des modules :
*******************************************************************
*/

// Initialisation de la carte SD
void initSD() 
{
  pinMode(SD_MISO_PIN, INPUT_PULLUP);
  SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  if (!SD.begin(SD_CS_PIN)) {
      Serial.println("Erreur de montage de la carte SD");
  } else {
      Serial.printf("Carte SD OK, taille: %uMB\n", SD.cardSize() / (1024 * 1024));
  }
}

void checkFactoryReset() {
  // Variable pour mesurer la durée d'appui
  unsigned long pressStartTime = 0;
  bool buttonPressed = false;

  pinMode(factoryResetPin, INPUT_PULLUP);


  // On vérifie si le bouton est enfoncé
  if (digitalRead(factoryResetPin) == LOW) {  // Bouton appuyé
    Serial.println("Appui long détecté au démarrage. Maintenez pour le reset usine...");
    pressStartTime = millis();  // Enregistre l'heure d'appui
    buttonPressed = true;

    // Boucle d'attente tant que le bouton est maintenu
    while (digitalRead(factoryResetPin) == LOW) {
      if (millis() - pressStartTime >= resetDelay) {
        factoryResetTriggered = true;  // Reset confirmé après l'appui long
        break;
      }
    }

    // Vérifie si l'appui long a bien été maintenu pour déclencher le reset usine
    if (factoryResetTriggered) {
      Serial.println("Appui long validé. Factory reset en cours...");
      if (config.resetToDefault("/config.json", "/default_config.json")) {
        Serial.println("Reset usine effectué. Configuration par défaut restaurée.");
      } else {
        Serial.println("Erreur lors du reset usine.");
      }
    } else {
      Serial.println("Appui long non maintenu. Chargement de la dernière configuration.");
      if (!config.load("/config.json")) {
        Serial.println("Erreur de chargement de la configuration, chargement des valeurs par défaut.");
      } else {
        Serial.println("Configuration chargée avec succès.");
      }
    }
  }
}