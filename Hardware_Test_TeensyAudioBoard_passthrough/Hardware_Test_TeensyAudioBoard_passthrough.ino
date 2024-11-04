/* Audio Passthrough example avec SGTL5000 (Teensy Audio Board)

This example code is in the public domain
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>

const int LED = 13; //LED interne

const int myInput = AUDIO_INPUT_LINEIN;
// const int myInput = AUDIO_INPUT_MIC;

// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
//

AudioInputI2S       audioInput;         // audio shield: mic or line-in
AudioOutputI2S      audioOutput;        // audio shield: headphones & line-out

// Créer les connexions entre entrée et sortie ("Passthrough")
//
AudioConnection c1(audioInput, 0, audioOutput, 0); // left passing through
AudioConnection c2(audioInput, 1, audioOutput, 1); // right passing through

// Définitions spécifiques du SGTL5000 pour choix de la source du casque
#define AUDIO_HEADPHONE_DAC 0
#define AUDIO_HEADPHONE_LINEIN 1

// Créer un objet pour contrôler le Teensy audio shield.
// 
AudioControlSGTL5000 audioShield;

void setup() {
  // Lancer un port série de controle sur USB
  Serial.begin(9600);
  while (!Serial) ;
  delay(3000);

// Allumer la LED du Teensy pour visualiser le fonctionnement
  Serial.println("Allumage de la LED");
  pinMode (LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Les connections audio ont besoin de mémoire pour fonctionner.
  // Pour plus d'informations, voir l'exemple "MemoryAndCpuUsage"
  Serial.println("Activation AudioMemory");
  AudioMemory(4);

  // Activer l'audio shield
  Serial.println("Activation du Shield Audio");
  audioShield.enable();
  Serial.println("Choix de l'entrée : Line Input");
  audioShield.inputSelect(myInput);
  Serial.println("Activation de la sortie Casque");
  audioShield.volume(0.7); //indispensable visiblement pour activer la sortie casque
  Serial.println("Niveaux i/o par défaut");
  Serial.println("Passthrough Lancé : Line In > Line Out + Headphones ");
}

void loop() {
 // Do nothing here.  The Audio flows automatically
}

