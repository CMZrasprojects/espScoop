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
#include <ESP32Ping.h>
//#include <Wire.h>
#include "SSD1306Wire.h"


/* Définition des Variables globales :
*******************************************************************
*/
// Variables pour l'écran OLED
int currentLine = 0;
const int maxLines = 6;  // Nombre de lignes sur l'écran
const int maxLineWidth = 128; // Nombre de pixels max en largeur
const int lineHeight = 10;  // Hauteur d'une ligne en pixels

// Variables globales Network sauvegardées
const char* deviceName = "name";
bool ethernetStatus = 0;
bool ethernetDhcp = 1;
IPAddress ethernetIpv4(0, 0, 0, 0);
IPAddress ethernetMasque(0, 0, 0, 0);
IPAddress ethernetPasserelle(0, 0, 0, 0);
bool wifiStatus = 0;
bool wifiDhcp = 0;
IPAddress wifiIpv4(0, 0, 0, 0);
IPAddress wifiMasque(0, 0, 0, 0);
IPAddress wifiPasserelle(0, 0, 0, 0);
bool lteStatus = 0;

// Variables globales Audio sauvegardées
float headphoneLevel = 0;
unsigned int sampleRate = 0;

// Déclaration de la liste des connexions WiFi sauvegardées
std::vector<WifiCredentials> wifiList;

// Variables globales au lancement (non sauvegardées)
const char* wifiSsid = "none";
const char*  wifiPassword = "none";
String husarnetIpv6 = "0000:0000:0000:0000:0000:0000:0000:0000";

// Variables pour validation des connexions réseau :
//unsigned long startTime;
//bool husarnetConnected = false;
IPAddress googleIP(8, 8, 8, 8); // Adresse IP de Google DNS, fiable pour le test de connexion

/* Définition des "Class" utilisées :
*******************************************************************
*/

sdConfigManager config;
HusarnetClient Husarnet;
WebServer server(80);

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

/* Fonctions
*******************************************************************
*/





/* Fonction Setup    *****************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
*/

void setup() {

  Serial.begin(115200);

  Serial.println("Initialisation de l'écran OLED");
  ecranInit();

  Serial.println("Initialisation de la Carte SD");
  initSD();
  delay(500);

  Serial.println("Verification d'une demande factory reset");
  checkFactoryReset();

  Serial.println("Verification de la liste Wifi");
  for (const WifiCredentials& creds : wifiList) {
    Serial.print("SSID : ");
    Serial.println(creds.ssid);
    Serial.print("Mot de passe : ");
    Serial.println(creds.password);
  }

  Serial.println("Tentative de connexion au réseau");
  connectToWiFi();
  //connectToEthernet();

  // Attendre la réponse du ping avec une limite de 5 secondes
  if (waitForInternetConnection(5000)) {
    Serial.println("Internet est accessible !");
    connectToHusarnet();
  } else {
    Serial.println("Pas de connexion Internet.");
  }

  Serial.println("Paramétrages du server web");
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on("/getStatus", HTTP_GET, getStatus);
  server.on("/setWiFiSettings", HTTP_POST, handleSetWiFiSettings);
  server.on("/setEthernetSettings", HTTP_POST, handleSetEthernetSettings);
  server.on("/setAudioSettings", HTTP_POST, handleSetAudioSettings);

  Serial.println("Démarrage du Server web");
  server.begin();

}
/* Fonction Loop
*************************************************************************************************************************************
*/

void loop() {

  server.handleClient(); // Permet de répondre aux requetes des clients (y compris 1ere connection !)
  delay(2);//allow the cpu to switch to other tasks

}
/* Fonctions avancées         ********************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
*/

// Fonctions pour envoyer les infos de connexions dans l'interface web lorsqu'un client les demande *********************
void getStatus()
{
    if (ETH.linkSpeed() != 0) { // CHANGER !!!!!!!!!!
      ethernetStatus = 1;
    } else {
      ethernetStatus = 0;
    }
    ethernetIpv4 = ETH.localIP();
    ethernetPasserelle = ETH.gatewayIP();
    ethernetMasque = ETH.subnetMask();

    wifiStatus = WiFi.isConnected();
    wifiIpv4 = WiFi.localIP();
    wifiPasserelle = WiFi.gatewayIP();
    wifiMasque = WiFi.subnetMask();
    wifiSsid = WiFi.SSID().c_str(); // Passage de String à const char* // Ca marche, mais j'arrive pas à le remettre en String après... Donc j'invoque WiFi.SSID() directement dans la reponse JSON
    husarnetIpv6 = Husarnet.getIpAddress().c_str(); //  ???????????????????!!!!!!!!!!!!!!!!!!!!!!!!! Ca marche !!!!!!!!!!!!!!!!!

    /* Conversion Data type vers String .............................
      bool: 
        String(variable); 
        String(variable ? "true" : "false");
      int, long, unsigned int, unsigned long :
        String(variable);
      float :
        String(variable);
        String(variable, 2);  // nombre de chiffres après la virgule
      char, [char], const char* :
        String(variable);
      IPAddress :
        variable.toString() 
      MOUAI..... Y'a des trucs bizarres avec les const char et/ou const char*
    */

    String jsonResponse = "{";
    jsonResponse += "\"deviceName\":\"" + String(deviceName) + "\",";
    jsonResponse += "\"ethernetStatus\":\"" + String(ethernetStatus) + "\",";
    jsonResponse += "\"ethernetDhcp\":\"" + String(ethernetDhcp) + "\",";
    jsonResponse += "\"ethernetIpv4\":\"" + ethernetIpv4.toString() + "\",";
    jsonResponse += "\"ethernetPasserelle\":\"" + ethernetPasserelle.toString() + "\",";
    jsonResponse += "\"ethernetMasque\":\"" + ethernetMasque.toString() + "\",";
    jsonResponse += "\"wifiStatus\":\"" + String(wifiStatus) + "\",";
    jsonResponse += "\"wifiDhcp\":\"" + String(wifiDhcp) + "\",";
    jsonResponse += "\"wifiIpv4\":\"" + wifiIpv4.toString() + "\",";
    jsonResponse += "\"wifiMasque\":\"" + wifiMasque.toString() + "\",";
    jsonResponse += "\"wifiPasserelle\":\"" + wifiPasserelle.toString() + "\",";
    jsonResponse += "\"wifiSsid\":\"" + WiFi.SSID() + "\",";
    jsonResponse += "\"husarnetIpv6\":\"" + husarnetIpv6 + "\",";
    jsonResponse += "\"sampleRate\":\"" + String(sampleRate) + "\",";
    jsonResponse += "\"headphoneLevel\":\"" + String(headphoneLevel) + "\"";
    
    jsonResponse += "}";

    server.send(200, "application/json", jsonResponse);
}
// Demande de modification des paramètres de connexions des flux AoIP
void handleSetConnexionSettings()
{
  ecran("Demande Client Web : Connexion Settings");
  server.send(200, "text/plain", "Connections settings updated");
  config.save("/config.json");
}
// Demande de modification des paramètres Audio
void handleSetAudioSettings()
{
  ecran("Demande Client Web : Audio Settings");
  sampleRate = server.arg("sampleRate").toInt();
  Serial.print("Nouvelle Frequence d'echantillonage après conversion .toInt() :");
  Serial.println(sampleRate);
  headphoneLevel = server.arg("headphoneLevel").toFloat();
  Serial.print("Nouveau volume casque :");
  Serial.println(headphoneLevel);

  server.send(200, "text/plain", "Audio settings updated");
  config.save("/config.json");

}
// Demande de modification de connexion au Wifi : Actif / SSID + Password / DHCP / IPs.  ************************************
void handleSetWiFiSettings()
{
  ecran("Demande Client Web : Wifi Settings");
  /* Conversions depuis des valeurs String récupérées depuis le client verx tous les datatype possibles :
  bool variableLocale = (server.arg("variableRecue") == "on"); ou bool variableLocale = (server.arg("variableRecue") == "1");
  int variableLocale = server.arg("variableRecue").toInt();
  float variableLocale = server.arg("variableRecue").toFloat();
  const char* variableLocale = (server.arg("variableRecue")).c_str();
  IPAddress variableLocale.fromString(server.arg("wifiIpv4"));

  Vaut mieux verifier que tout se passe bien, par exemple pour IPAddress :
  IPAddress variableLocale;  // si la variable n'a pas déjà été déclaré
  if (!variableLocale.fromString(server.arg("variableRecue"))) {
    // Gérer l'erreur
  }
  */
    wifiStatus = (server.arg("wifiStatus") == "on");  // Récupère l'état du Wi-Fi (on/off)   
    Serial.print("wifiStatus :");
    Serial.println(wifiStatus);  
    const char* newWifiSsid = (server.arg("newWifiSsid")).c_str();        // Récupère le SSID
    Serial.print("Nouveau SSID :");
    Serial.println(newWifiSsid);
    const char* newWifiPassword = (server.arg("newWifiPassword")).c_str(); // Récupère le mot de passe
    Serial.print("Nouveau Mot de passe :");
    Serial.println(newWifiPassword);
    wifiDhcp = (server.arg("wifiDhcp") == "on");      // Récupère l'état du DHCP sur Wi-Fi (on/off)
    Serial.print("wifiDhcp :");
    Serial.println(wifiDhcp);
    wifiIpv4.fromString(server.arg("wifiIpv4")); // Récupère l'ipv4 
    Serial.print("wifiIpv4 :");
    Serial.println(wifiIpv4);         
    wifiMasque.fromString(server.arg("wifiMasque"));      // Récupère le masque
    Serial.print("wifiMasque :");
    Serial.println(wifiMasque);
    wifiPasserelle.fromString(server.arg("wifiPasserelle")); // Récupère la passerelle
    Serial.print("wifiPasserelle :");
    Serial.println(wifiPasserelle);

    if (wifiStatus == 1) {
      if (wifiDhcp == 1){
        WiFi.disconnect(); // Déconnexion du Wi-Fi
        WiFi.begin(newWifiSsid, newWifiPassword); // Connexion au Wi-Fi
      } else  {
        WiFi.disconnect(); // Déconnexion du Wi-Fi
        WiFi.config(IPAddress(wifiIpv4), IPAddress(wifiPasserelle), IPAddress(wifiMasque), IPAddress(wifiPasserelle)); // On impose ipv4, passerelle, masque, dns1
        WiFi.begin(newWifiSsid, newWifiPassword); // Connexion au Wi-Fi
      }
    }

    server.send(200, "text/plain", "WiFi settings updated");
    config.save("/config.json");
}
void handleSetEthernetSettings()
{
  ecran("Demande Client Web : Ethernet Settings");
  /* Conversions depuis des valeurs String récupérées depuis le client verx tous les datatype possibles :
  bool variableLocale = (server.arg("variableRecue") == "on"); ou bool variableLocale = (server.arg("variableRecue") == "1");
  int variableLocale = server.arg("variableRecue").toInt();
  float variableLocale = server.arg("variableRecue").toFloat();
  const char* variableLocale = (server.arg("variableRecue")).c_str();
  IPAddress variableLocale.fromString(server.arg("wifiIpv4"));

  Vaut mieux verifier que tout se passe bien, par exemple pour IPAddress :
  IPAddress variableLocale;  // si la variable n'a pas déjà été déclaré
  if (!variableLocale.fromString(server.arg("variableRecue"))) {
    // Gérer l'erreur
  }
  */
    //ethernetStatus = (server.arg("ethernetStatus") == "on");  // Récupère l'état de l'ethernet (on/off)   
    //Serial.print("ethernetStatus :");
    //Serial.println(ethernetStatus);  
    ethernetDhcp = (server.arg("ethernetDhcp") == "on");      // Récupère l'état du DHCP de l'ethernet(on/off)
    Serial.print("ethernetDhcp :");
    Serial.println(ethernetDhcp);
    ethernetIpv4.fromString(server.arg("ethernetIpv4")); // Récupère l'ipv4 
    Serial.print("ethernetIpv4 :");
    Serial.println(ethernetIpv4);         
    ethernetMasque.fromString(server.arg("ethernetMasque"));      // Récupère le masque
    Serial.print("ethernetMasque :");
    Serial.println(ethernetMasque);
    ethernetPasserelle.fromString(server.arg("ethernetPasserelle")); // Récupère la passerelle
    Serial.print("ethernetPasserelle :");
    Serial.println(ethernetPasserelle);

    if (ethernetDhcp == 1){
      Serial.print("Ethernet : Configuration en DHCP");
      Serial.print("Valeur ethernetDhcp :");
      Serial.println(ethernetDhcp);
      ETH.config(IPAddress(0, 0, 0, 0));
    } else  {
      Serial.print("Valeur ethernetDhcp :");
      Serial.println(ethernetDhcp);
      Serial.print("Ethernet : Configuration en IP fixe");
      Serial.print("IPAddress(ethernetIpv4) :");
      Serial.println(IPAddress(ethernetIpv4));
      ETH.config(IPAddress(ethernetIpv4), IPAddress(ethernetPasserelle), IPAddress(ethernetMasque)); // On impose ipv4, passerelle, masque, dns1
    }
    ecran("Nouvelle adresse IP Eth :" + (ETH.localIP()).toString());
    server.send(200, "text/plain", "Ethernet settings updated");
    config.save("/config.json");
}
/* Fonctions de base : initialisation des modules :   ********************************************************************************
**************************************************************************************************************************************
**************************************************************************************************************************************
*/
// Initialisation et fonctions de l'ecran OLED  retour à la ligne automatique et gestion de débordement ******************************
void ecranInit() {
  // Initialising the UI will init the display too.
  display.init();
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  ecran("Ecran Oled OK");
}
// Affichage complet :
void ecranFull() {
  display.clear();
  currentLine = 0;
  display.drawString(0, 0, "Hello world"); // (AlignementHorizontal, alignementVertical, "Texte...")

}
/* Affichage ligne par ligne ; Exemple d'utilisation :
Comprenant au choix texte et variable de n'importe quel type, dans n'importe quel ordre... + Renvoi à la ligne automatique (2 lignes max ???) :
  ecran("Valeur de ethernetIpv4 : " + ethernetIpv4.toString());
  ecran("Valeur de ethernetDhcp : " + String(ethernetDhcp));
*/
void ecran(const String& text) {
  int start = 0;
  String line = "";
  int linesNeeded = 0;  // Compteur de lignes nécessaires pour le texte restant

  // Calcul du nombre total de lignes nécessaires pour le texte
  for (int i = 0; i < text.length(); i++) {
    line += text[i];
    if (display.getStringWidth(line) >= maxLineWidth) {
      linesNeeded++;
      line = "";
    }
  }
  // Ajoute une ligne si un reste de texte n'atteignant pas maxLineWidth est présent
  if (line.length() > 0) linesNeeded++;

  // Si le texte nécessite plus de lignes que disponibles, efface l'écran et recommence en haut
  if (linesNeeded > (maxLines - currentLine)) {
    display.clear();
    currentLine = 0;
  }

  // Réinitialisation pour l'affichage réel, avec retour à la ligne
  line = "";
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    line += c;
    int lineWidth = display.getStringWidth(line);

    // Affiche la ligne si elle atteint la largeur maximale
    if (lineWidth >= maxLineWidth) {
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(0, currentLine * lineHeight, line);
      display.display();

      currentLine++;
      if (currentLine >= maxLines) {
        display.clear();
        currentLine = 0;
      }
      line = "";  // Réinitialise la ligne pour les caractères restants
    }
  }

  // Affiche le reste du texte s'il en reste après la dernière ligne complète
  if (line.length() > 0) {
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, currentLine * lineHeight, line);
    display.display();
    currentLine++;

    if (currentLine >= maxLines) {
      display.clear();
      currentLine = 0;
    }
  }
}

// Initialisation de la carte SD  ***************************************************************************************
void initSD() 
{
  pinMode(SD_MISO_PIN, INPUT_PULLUP);
  SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  if (!SD.begin(SD_CS_PIN)) {
      ecran("Erreur de Carte SD");
      Serial.println("Erreur de montage de la carte SD");
  } else {
      ecran("Carte SD OK");
      Serial.printf("Carte SD OK, taille: %uMB\n", SD.cardSize() / (1024 * 1024));
  }
}

// Fonction de réinitialisation des paramètres aux réglages usine  stockés dans default_config.json ***********************
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
      ecran("Appui long détecté...");
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
        ecran("Reset en cours...");
        Serial.println("Appui long validé. Lancement du Factory reset en cours...");
        if (config.resetToDefault("/default_config.json", "/config.json")) {
          ecran("Reset OK");
          Serial.println("Reset usine effectué. Configuration par défaut restaurée depuis default_config.json.");
        } else {
          Serial.println("Erreur lors du reset usine.");
        }
      } else {
        Serial.println("Appui long non maintenu. Chargement de la configuration depuis config.json.");
        if (!config.load("/config.json")) {
          Serial.println("Erreur de chargement de la configuration, chargement des valeurs par défaut.");
        } else {
          ecran("Config OK");
          Serial.println("Fin de la sequence reset");
        }
      }
      // On sort de la fonction après le reset
      return;
    }
  }
  // Si aucun appui n'est détecté au démarrage, charger la configuration normalement
  ecran("Chargement config...");
  Serial.println("Aucun appui détecté. Chargement de la dernière configuration depuis config.json.");
  if (!config.load("/config.json")) {
    Serial.println("Erreur de chargement de la configuration, chargement des valeurs par défaut.");
  } else {
    ecran("Config OK");
    Serial.println("Fin de la séquence reset");
  }
}
// Fonction pour démarrer le port Ethernet
void connectToEthernet(){
  // Si on est en DHCP, on renseigne l'ipv4 avec 0.0.0.0
  if (ethernetDhcp == 1) {
      Serial.println("Ethernet : Configuration en DHCP");
      ETH.config(IPAddress(0, 0, 0, 0));
  } else {
      Serial.println("Ethernet : Configuration en IP fixe");
      Serial.print("ethernetIpv4 :");
      Serial.println(ethernetIpv4);
      Serial.print("IPAddress(ethernetIpv4) :");
      Serial.println(IPAddress(ethernetIpv4));
      ETH.config(IPAddress(ethernetIpv4), IPAddress(ethernetPasserelle), IPAddress(ethernetMasque));
  }
#if CONFIG_IDF_TARGET_ESP32
    if (!ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN,
                   ETH_MDIO_PIN, ETH_RESET_PIN, ETH_CLK_MODE)) {
        Serial.println("ETH start Failed!");
    }
#else
    if (!ETH.begin(ETH_PHY_W5500, 1, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN,
                   SPI3_HOST,
                   ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN)) {
        Serial.println("ETH start Failed!");
    }
    else {
      Serial.println("Module Ethernet W5500 chargé");
      ecran("Ethernet OK :" + (ETH.localIP()).toString());
    }
#endif

  if (ETH.config(IPAddress(ethernetIpv4), IPAddress(ethernetPasserelle), IPAddress(ethernetMasque)) == false) {
    Serial.println("Configuration failed.");
  }


}

// Fonction pour se connecter au Wi-Fi en fonction des réseaux sauvegardés dans wifiList.  *************************
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
                Serial.printf("Avec ce mot de passe : %s\n", creds.password);
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
                    ecran("Connexion Wifi OK :" + (WiFi.localIP()).toString());
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
// Vérifier la connexion à Internet avant de lancer husarnet en pingant Google !     ***********************
bool waitForInternetConnection(unsigned long timeout) {
    unsigned long startTime = millis();
    Serial.println("Vérification de la connexion Internet...");

    // Boucle pour vérifier plusieurs fois jusqu'au timeout
    while ((millis() - startTime) < timeout) {
        if (Ping.ping(googleIP)) {
            ecran("Connexion Internet OK");
            return true;  // Connexion réussie
        }
        delay(500);  // Attendre avant de retenter (pour éviter de spammer le ping)
    }
    ecran("Pas de connexion Internet !");
    return false;  // Timeout atteint sans réponse du ping
}
// Fonction d'initialisation et connexion à Husarnet ********************************************************
void connectToHusarnet()
{
  // Joindre Husarnet, avec une petite attente pour l'initialisation
  // delay(1000);  // délai d'attente avant la tentative de connexion
  Husarnet.join(deviceName, "fc94:b01d:1803:8dd8:b293:5c7d:7639:932a/iN4FhatGTppdpqNWQ4DS3M");
  ecran("Connexion à Husarnet...");

  while (!Husarnet.isJoined()) {
    Serial.println("En attente de la connexion au réseau Husarnet...");
    delay(2000);  // vérifier toutes les secondes
  }

  if (Husarnet.isJoined()) {
    Serial.println("Connexion au réseau Husarnet réussie !");
    ecran("Connexion Husarnet active !");
    ecran("Taper http://" + String(deviceName));
    Serial.print("Husarnet IP: ");
    Serial.println(Husarnet.getIpAddress().c_str());
    husarnetIpv6 = Husarnet.getIpAddress().c_str();
  } else {
    Serial.println("Échec de connexion au réseau Husarnet après plusieurs tentatives.");
    // Ajouter ici une gestion d'erreur si nécessaire
  }
}

// Fonction pour démarrer le webServer à partir du fichier index.html sur la carte SD.  *************************
void handleRoot()
{
  ecran("Un client Web connecté !");
  File file = SD.open("/index.html");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}
// Fonction pour renvoyer au client un message d'erreur lorsque la requête ne peut être remplie.  ****************
void handleNotFound()
{
    ecran("Erreur de requete HTTP");
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

/* ***************************************************************
******************************************************************
******************************************************************
******************************************************************
*/
/*
    Dans le setup, placer : startConnectionCycle();

void loop() {
    // Vérifie si la connexion Husarnet est active, sinon relance le cycle de connexion
    if (husarnetConnected && !isNetworkConnected()) {
        Serial.println("Connexion perdue, relance du cycle de connexion...");
        husarnetConnected = false;
        startConnectionCycle();
    }
}

void startConnectionCycle() {
    while (!husarnetConnected) {
        if (tryEthernetConnection()) {
            connectToHusarnet();
        } else if (tryWifiConnection()) {
            connectToHusarnet();
        } else {
            Serial.println("Aucune connexion établie, nouvelle tentative dans 15 secondes...");
            delay(15000);
        }
    }
}

bool tryEthernetConnection() {
    Serial.println("Tentative de connexion Ethernet...");
    startTime = millis();
    connectToEthernet();

    while (millis() - startTime < 3000) {
        if (isEthernetConnected()) {
            Serial.println("Connexion Ethernet réussie");
            return true;
        }
    }
    Serial.println("Échec de la connexion Ethernet, tentative de connexion Wi-Fi...");
    return false;
}

bool tryWifiConnection() {
    Serial.println("Tentative de connexion Wi-Fi...");
    connectToWifi();
    startTime = millis();

    while (millis() - startTime < 3000) {
        if (isWifiConnected()) {
            Serial.println("Connexion Wi-Fi réussie");
            return true;
        }
    }
    Serial.println("Échec de la connexion Wi-Fi");
    return false;
}

void connectToHusarnet() {
    Serial.println("Connexion à Husarnet...");
    connectToHusarnet();
    startTime = millis();

    while (millis() - startTime < 5000) {
        if (isHusarnetConnected()) {
            Serial.println("Connexion Husarnet réussie");
            husarnetConnected = true;
            return;
        }
    }
    Serial.println("Échec de la connexion Husarnet");
}

// Fonctions d'état pour vérifier les connexions
bool isEthernetConnected() {
    // Vérifiez l'état de la connexion Ethernet ici
}

bool isWifiConnected() {
    // Vérifiez l'état de la connexion Wi-Fi ici
}

bool isHusarnetConnected() {
    // Vérifiez l'état de la connexion Husarnet ici
}

bool isNetworkConnected() {
    return isEthernetConnected() || isWifiConnected();
}
*/