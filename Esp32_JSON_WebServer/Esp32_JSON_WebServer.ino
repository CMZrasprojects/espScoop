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

// Déclaration de la liste des connexions WiFi sauvegardées
std::vector<WifiCredentials> wifiList;

/* Définition des "Class" utilisées :
*******************************************************************
*/

sdConfigManager config;
HusarnetClient Husarnet;
WebServer server(80);

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

  Serial.println("Verification d'une demande factory reset, sinon chargement de la derniere configuration");
  checkFactoryReset();

  // Maintenant, tu peux accéder à wifiList dans ton sketch
  Serial.println("Verification de la liste Wifi");
  for (const WifiCredentials& creds : wifiList) {
    Serial.print("SSID : ");
    Serial.println(creds.ssid);
    Serial.print("Mot de passe : ");
    Serial.println(creds.password);
  }

  connectToWiFi();
  connectToHusarnet();

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("WebServeur démarré");


}
/* Fonction Loop
*******************************************************************
*/

void loop() {

  server.handleClient(); // Permet de répondre aux requetes des clients (y compris 1ere connection !)
  delay(2);//allow the cpu to switch to other tasks

}

/* Fonctions de base : initialisation des modules :
**********************************************************************************************
**********************************************************************************************
**********************************************************************************************
**********************************************************************************************
**********************************************************************************************
*/

// Initialisation de la carte SD  ************************************************************
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

// Check demande de réinitialisation des paramètres aux réglages usine  ***********************
void checkFactoryReset() {

  // Attente initiale pour détecter un appui long au démarrage
  unsigned long startWaitTime = millis();

  const int factoryResetPin = 46; // Pin du bouton pour le reset usine
  const unsigned long resetDelay = 5000;  // Durée (en ms) d'appui pour déclencher le reset
  bool factoryResetTriggered = false;

  // Variable pour mesurer la durée d'appui
  unsigned long pressStartTime = 0;
  bool buttonPressed = false;

  pinMode(factoryResetPin, INPUT_PULLUP);

  // Boucle d'attente (ex. 500 ms) pour voir si un appui est détecté
  while (millis() - startWaitTime < 500) {
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
        if (config.resetToDefault("/default_config.json", "/config.json")) {
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
      // On sort de la fonction après le reset
      return;
    }
  }
  // Si aucun appui n'est détecté au démarrage, charger la configuration normalement
  Serial.println("Aucun appui détecté. Chargement de la dernière configuration.");
  if (!config.load("/config.json")) {
    Serial.println("Erreur de chargement de la configuration, chargement des valeurs par défaut.");
  } else {
    Serial.println("Configuration chargée avec succès.");
  }
}

// Fonction pour se connecter au Wi-Fi en fonction des réseaux sauvegardés dans wifiList
void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    // Scanner les réseaux Wi-Fi disponibles
    int numNetworks = WiFi.scanNetworks();
    Serial.println("Scan terminé");

    // Parcourir les réseaux détectés
    for (int i = 0; i < numNetworks; i++) {
        const char* ssid = WiFi.SSID(i).c_str();  // Récupérer le SSID du réseau détecté
        Serial.print("Récupération du ssid du réseau le plus puissant : ");
        Serial.println(ssid);

        // Parcourir la liste wifiList et chercher un réseau avec le même SSID
        for (const WifiCredentials& creds : wifiList) {
              if (strcmp(ssid, creds.ssid) == 0) {  // Comparaison des chaînes C
                Serial.printf("Connexion au réseau WiFi le plus puissant : %s\n", ssid);
                WiFi.begin(ssid, creds.password); // Utiliser directement creds.password
                int tries = 0;

                // Attendre que la connexion soit établie
                while (WiFi.status() != WL_CONNECTED && tries < 20) {
                    delay(500);
                    Serial.print(".");
                    tries++;
                }

                // Vérifier si la connexion est réussie
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("");
                    Serial.print("Connecté au Wi-Fi : ");
                    Serial.println(ssid);
                    Serial.print("Adresse IP : ");
                    Serial.println(WiFi.localIP());
                    return;  // Connexion réussie, sortir de la fonction
                } else {
                    Serial.println("Échec de la connexion.");
                }
            }
        }
    }

    // Si aucun réseau connu n'est trouvé, afficher un message
    Serial.println("Aucun réseau connu trouvé. Tentative de connexion échouée.");
}

// Fonction d'initialisation et connexion à Husarnet **********************************************
void connectToHusarnet()
{
  // Joindre Husarnet, avec une petite attente pour l'initialisation
  delay(1000);  // délai d'attente avant la tentative de connexion
  Husarnet.join("esp32-Cmz-1", "fc94:b01d:1803:8dd8:b293:5c7d:7639:932a/iN4FhatGTppdpqNWQ4DS3M");

  while (!Husarnet.isJoined()) {
    Serial.println("En attente de la connexion au réseau Husarnet...");
    delay(2000);  // vérifier toutes les secondes
  }

  if (Husarnet.isJoined()) {
    Serial.println("Connexion au réseau Husarnet réussie !");
    Serial.print("Husarnet IP: ");
    Serial.println(Husarnet.getIpAddress().c_str());
  } else {
    Serial.println("Échec de connexion au réseau Husarnet après plusieurs tentatives.");
    // Ajouter ici une gestion d'erreur si nécessaire
  }
}

// Fonction pour démarrer le webServer à partir du fichier index.html sur la carte SD
void handleRoot()
{
  File file = SD.open("/index.html");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}