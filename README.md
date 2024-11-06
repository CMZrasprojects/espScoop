# espScoop
## Présentation

Emetteur/Récepteur AoIP embarqué dans un MCU, sur LAN privé ou Internet via tunneling VPN. 
Connexion par Wifi / Ethernet / 4G LTE Cat 4. Alimentation batterie.

Entrée/sortie audio analogiques niveau Ligne + sortie Casque avec fonctions basiques de mixage.

Contrôle complet des paramètres audio et de connectivité par une page web accessible en local ou à distance.

## Hardware
 
MCU : 
   - Lilygo T-Eth-Lite (esp32-s3-WROOM-1)
      240 MHz / 16MB Flash / 8MB PSRAM / uSD / PoE
      Wifi / BLE / Ethernet (W5500 SPI) / i2c / i2s
      ESP-IDF / ESP-ADF

   - Teensy 4.1 + extension ethernet ?

   - Esp32 + Teensy / DaisySeed ?

   - Esp32 Audio Dev Kit ?

   - Esp32 4G LTE Dev Kit ?

Connectivité 4G LTE Cat 4 : 
    - Hat esp32 / Hat Raspberry ?
    - Réseau via SPI ou TinyUsb ?
    - Sur MCU Additionnel ou le même ?
        
Ecran Controle OLED (option) : 64x64 i2c
 Boutons : 
    - On / Off
    - Controle d'un menu sur OLED (option)
  

Connectivité Audio :
     - Teensy Audio Board :
         Codec SGTL5000 / Contrôle i2c / Audio i2s
         Entrées et sorties Ligne Stéréo
         Sortie Casque + Possibilité potentiomètre
         ⚠️ 16bits / 44,1 kHz
         Carte SD SPI / Extension RAM ?
      - A Trouver : Board 24bits / 48 kHz / Analogique ou AES

Alimentation :
      - 5V sur Batterie USB
      - 12V sur XLR-4 ou Hirose-4 + Convertisseur 12V-5V

## Software
 - Arduino IDE <<<<
    > pratique et + comprehensible
 - Visual Studio
    > usine a gaz... puissant... compilation + rapide... des heures a etudier le manuel
 - idf.py
    > yen a marre des lignes de code
    
### Audio
Communication avec la Board : AudioTools pour Arduino
Conception Audio (mixage & AoIP) : PureData pour Arduino
 - espd
 - hvcc
    Record local ?
    
### AoIP
Audio Over OSC (AOO)
 Bibliotheque C/C++
 Bibliotheque esp32 (en developpement)
 Bibliotheque external PureData

Connexions à la volée et multiples, choix du CODEC, adaptation du buffer, serveur 

Monitoring sur la page Web ?
  
### Réseau
WebServer 
VPN Husarnet
Page html : Visualisation et changement des paramètres ...
 - Réseau
 - Audio Board : Niveaux, Mixage
  - AoIP : Encodage, Canaux, host, ID, Buffer
  - Liste des appareils en réseau
  - Raccourcis de connexions entre appareils (Grille ?)
                    
Menu CSS / JS ; façon uScoop
Stockage sur la uSD
      
### Page Web <> Esp32
    html Get / Post + JSON <<<<<

### Sauvegarde des paramètres
 - Bibliotheque Arduino IDE "Preferences" > NVS
 - JSON sur la carte SD <<<<
    
# Developpement
## Audio Board :
### Teensy + Teensy Audio Board + Arduino IDE
 Test de base OK : Sketch Hardware_Test_TeensyAudioBoard_passthrough
### Esp32 + Teensy Audioboard
 Pas de librairie trouvé pour le SGTL5000
 Essai avec https://github.com/sinneb/esp32sampler adapté pour "esp32s3box"
 Essai d'integration direct du driver
### Tests 48 kHz
### Tests PureData sur esp32
espd ou hvcc
### Conception Mixeur
### Code Complet

## Audio over IP
### Essai Intégration bibliotheque PD external
### Essai aoo/esp32

## Réseau
WebServer OK
Husarnet OK
Page Html sur SD OK
Navigation OK
Fichier d'échange JSON OK
 Mettre a jour et tester recuperation + sauvegarde en fonction des avancees sur la sauvegarde en JSON


## Sauvegarde
Paramètres Réseau (preference nvs) OK
 Facile, mais 1/2ko max
 
Paramètres complets (JSON sur Sd) -- EN COURS

 Taille infinie selon SD
 Creer un config.json + default_config.json pour "factory reset"
 Bibliotheque ArduinoJson.h
 
 Fonction readall() + Fonction write(parametre, valeur) pour chaque data_type
 
 OU C++ structure
  Pas beaucoup plus simple, sauvegarde de tous les parametres a chaque fois
  
 OU Classe de configuration (-> creation dune bibliotheque ?)
  plus fastidieux mais code plus clair, et personnalisavle et evolutif
