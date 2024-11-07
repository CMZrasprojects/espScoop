/**
 * @file    WebServer_html_SD_Pref     
 * @author  C.Moizeau   
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
#include <Preferences.h>        // Config pour persistance des reglages IP
#include <husarnet.h>

//Change to IP and DNS corresponding to your network, gateway
IPAddress staticIP(192, 168, 42, 91);
IPAddress gateway(192, 168, 42, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 42, 1);

static bool eth_connected = false;

HusarnetClient Husarnet;

WebServer server(80);

Preferences preferences;

// Sauvegarde des adresses IP
void saveNetworkSettings(IPAddress ip, IPAddress gw, IPAddress sn)
{
    preferences.begin("network", false);
    preferences.putUInt("ip0", ip[0]);
    preferences.putUInt("ip1", ip[1]);
    preferences.putUInt("ip2", ip[2]);
    preferences.putUInt("ip3", ip[3]);
    preferences.putUInt("gw0", gw[0]);
    preferences.putUInt("gw1", gw[1]);
    preferences.putUInt("gw2", gw[2]);
    preferences.putUInt("gw3", gw[3]);
    preferences.putUInt("sn0", sn[0]);
    preferences.putUInt("sn1", sn[1]);
    preferences.putUInt("sn2", sn[2]);
    preferences.putUInt("sn3", sn[3]);
    preferences.end();
}

// Chargement des adresses IP
void loadNetworkSettings(IPAddress &ip, IPAddress &gw, IPAddress &sn) {
    preferences.begin("network", true);
    ip[0] = preferences.getUInt("ip0", 0);
    ip[1] = preferences.getUInt("ip1", 0);
    ip[2] = preferences.getUInt("ip2", 0);
    ip[3] = preferences.getUInt("ip3", 0);
    gw[0] = preferences.getUInt("gw0", 0);
    gw[1] = preferences.getUInt("gw1", 0);
    gw[2] = preferences.getUInt("gw2", 0);
    gw[3] = preferences.getUInt("gw3", 0);
    sn[0] = preferences.getUInt("sn0", 0);
    sn[1] = preferences.getUInt("sn1", 0);
    sn[2] = preferences.getUInt("sn2", 0);
    sn[3] = preferences.getUInt("sn3", 0);
    preferences.end();
}


void handleRoot()
{

    if (server.hasArg("ip0") && server.hasArg("ip1") && server.hasArg("ip2") && server.hasArg("ip3")) {
        IPAddress newIP(
            server.arg("ip0").toInt(),
            server.arg("ip1").toInt(),
            server.arg("ip2").toInt(),
            server.arg("ip3").toInt()
        );
        IPAddress newGateway(
            server.arg("gateway0").toInt(),
            server.arg("gateway1").toInt(),
            server.arg("gateway2").toInt(),
            server.arg("gateway3").toInt()
        );
        IPAddress newSubnet(
            server.arg("subnet0").toInt(),
            server.arg("subnet1").toInt(),
            server.arg("subnet2").toInt(),
            server.arg("subnet3").toInt()
        );

        // Sauvegarder les nouvelles configurations IP
        saveNetworkSettings(newIP, newGateway, newSubnet);

        // Appliquer la configuration réseau en fonction des valeurs IP
        if (newIP == IPAddress(0, 0, 0, 0)) {
            ETH.config(IPAddress(0, 0, 0, 0)); // DHCP si IP à 0.0.0.0
            Serial.println("Handle Root : Mode DHCP activé.");
        } else {
            ETH.config(newIP, newGateway, newSubnet); // IP fixe sinon
            Serial.println("Handle Root : Configuration IP fixe appliquée.");
        }
    }

    

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

void handleSetNetwork()
{
    if (server.hasArg("dhcp") && server.arg("dhcp") == "on") {
        // Mode DHCP, enregistrement de l'adresse 0.0.0.0
        ETH.config(IPAddress(0, 0, 0, 0)); 
        Serial.println("handleSetNetwork : Configuration en mode DHCP");
        
        // Sauvegarde des adresses à 0.0.0.0 pour indiquer le DHCP
        saveNetworkSettings(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0));
        Serial.println("handleSetNetwork : Configuration DHCP enregistrée");

    } else {
        // Configuration en IP fixe
        Serial.println("handleSetNetwork : Mode IP fixe sélectionné, vérification des valeurs :");
        
        int ip0 = server.arg("ip0").toInt();
        int ip1 = server.arg("ip1").toInt();
        int ip2 = server.arg("ip2").toInt();
        int ip3 = server.arg("ip3").toInt();
        
        int gw0 = server.arg("gateway0").toInt();
        int gw1 = server.arg("gateway1").toInt();
        int gw2 = server.arg("gateway2").toInt();
        int gw3 = server.arg("gateway3").toInt();
        
        int sn0 = server.arg("subnet0").toInt();
        int sn1 = server.arg("subnet1").toInt();
        int sn2 = server.arg("subnet2").toInt();
        int sn3 = server.arg("subnet3").toInt();

        // Afficher les valeurs récupérées pour débogage
        Serial.printf("IP: %d.%d.%d.%d\n", ip0, ip1, ip2, ip3);
        Serial.printf("Gateway: %d.%d.%d.%d\n", gw0, gw1, gw2, gw3);
        Serial.printf("Subnet: %d.%d.%d.%d\n", sn0, sn1, sn2, sn3);

        IPAddress newIP(ip0, ip1, ip2, ip3);
        IPAddress newGateway(gw0, gw1, gw2, gw3);
        IPAddress newSubnet(sn0, sn1, sn2, sn3);

        ETH.config(newIP, newGateway, newSubnet);

        // Sauvegarder les paramètres dans la mémoire
        saveNetworkSettings(newIP, newGateway, newSubnet);
        Serial.printf("handleSetNetwork : Configuration IP fixe enregistrée et appliquée: %s\n", newIP.toString().c_str());
    }
    
    server.send(200, "text/plain", "Configuration mise à jour");
}



void WiFiEvent(arduino_event_id_t event)
{
  switch (event) {
  case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
  case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
  case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
          Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
  case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
  default:
      break;
  }
}
/* A quoi ça sert ça ???!!!!! Tester la connexion d'un client au server ?
void testClient(const char *host, uint16_t port)
{
    Serial.print("\nconnecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    while (client.connected() && !client.available())
        ;
    while (client.available()) {
        Serial.write(client.read());
    }

    Serial.println("closing connection\n");
    client.stop();
}
*/

// Fonctions pour récupérer les infos de connexions dans l'interface web
void getNetworkStatus()
{
    IPAddress ethIP = ETH.localIP();
    IPAddress ethGateway = ETH.gatewayIP();
    IPAddress ethSubnet = ETH.subnetMask();

    IPAddress wifiIP = WiFi.localIP();
    IPAddress wifiGateway = WiFi.gatewayIP();
    IPAddress wifiSubnet = WiFi.subnetMask();
    String wifiSSID = WiFi.SSID();

    String jsonResponse = "{";
    jsonResponse += "\"ethIP\":\"" + ethIP.toString() + "\",";
    jsonResponse += "\"ethGateway\":\"" + ethGateway.toString() + "\",";
    jsonResponse += "\"ethSubnet\":\"" + ethSubnet.toString() + "\",";
    jsonResponse += "\"wifiIP\":\"" + wifiIP.toString() + "\",";
    jsonResponse += "\"wifiGateway\":\"" + wifiGateway.toString() + "\",";
    jsonResponse += "\"wifiSubnet\":\"" + wifiSubnet.toString() + "\",";
    jsonResponse += "\"wifiSSID\":\"" + wifiSSID + "\"";
    jsonResponse += "}";

    server.send(200, "application/json", jsonResponse);
}

// Fonction pour gérer l'enregistrement du SSID et du mot de passe
void handleSetWiFiSettings()
{
    String ssid = server.arg("ssid");        // Récupère le SSID
    String password = server.arg("password"); // Récupère le mot de passe
    String status = server.arg("status");      // Récupère l'état du Wi-Fi (on/off)

    if (status == "on") {
        WiFi.begin(ssid.c_str(), password.c_str()); // Connexion au Wi-Fi
    } else {
        WiFi.disconnect(); // Déconnexion du Wi-Fi
    }

    server.send(200, "text/plain", "WiFi settings updated");
}


void setup()
{
  Serial.begin(115200);

  // Chargement des paramètres sauvegardés
  loadNetworkSettings(staticIP, gateway, subnet);

  // Si toutes les valeurs sont 0.0.0.0, passage en DHCP
  if (staticIP == IPAddress(0, 0, 0, 0)) {
      Serial.println("Setup : Passage en DHCP");
      ETH.config(IPAddress(0, 0, 0, 0));
  } else {
      Serial.println("Setup : Passage en IP fixe");
      ETH.config(staticIP, gateway, subnet);
  }

  WiFi.mode(WIFI_STA);

  // Initialisation de la carte SD
  pinMode(SD_MISO_PIN, INPUT_PULLUP);
  SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  if (!SD.begin(SD_CS_PIN)) {
      Serial.println("Erreur de montage de la carte SD");
  } else {
      Serial.printf("Carte SD OK, taille: %uMB\n", SD.cardSize() / (1024 * 1024));
  }

  WiFi.onEvent(WiFiEvent);

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
      Serial.println("Module ETH W5500 chargé");
    }
#endif

    if (ETH.config(staticIP, gateway, subnet, dns, dns) == false) {
        Serial.println("Configuration failed.");
    }

    // Configuration des identifiants WiFi temporaires
    const char* ssid = "iPhone de Clement";       // Remplacez par votre SSID temporaire
    const char* password = "clementiphone"; // Remplacez par votre mot de passe temporaire

    // Connexion au WiFi avec les identifiants temporaires
    WiFi.begin(ssid, password);
    WiFi.enableIPv6(true);
    Serial.print("Connexion au WiFi...");
    
    unsigned long startTime = millis();
    const unsigned long timeout = 10000; // Temps d'attente max de 10 secondes

    // Attente de la connexion WiFi ou expiration
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(5000);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connecté!");
        Serial.print("Adresse IP : ");
        Serial.println(WiFi.localIP());
        Serial.print("Adresse local IPv6 : ");
        Serial.println(WiFi.linkLocalIPv6());
        Serial.print("Adresse global IPv6 : ");
        Serial.println(WiFi.globalIPv6());

        connectToHusarnet();

        // Déconnexion du WiFi après connexion à Husarnet
        WiFi.disconnect();
        Serial.println("WiFi déconnecté après connexion à Husarnet.");
    } else {
        Serial.println("\nErreur : Impossible de se connecter au WiFi dans le délai imparti.");
    }

    connectToHusarnet();

    server.on("/", handleRoot);

    server.on("/setNetwork", HTTP_POST, handleSetNetwork);

    server.on("/getNetworkStatus", HTTP_GET, getNetworkStatus);

    server.on("/setWiFiSettings", HTTP_POST, handleSetWiFiSettings);

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");    

}


void loop()
{
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}

void connectToHusarnet()
{
  // Joindre Husarnet, avec une petite attente pour l'initialisation
  delay(5000);  // délai d'attente avant la tentative de connexion
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