// configVariables.h
#ifndef CONFIGVARIABLES_H
#define CONFIGVARIABLES_H

// Variables globales Network
extern const char* deviceName;
extern bool ethernetStatus;
extern bool ethernetDhcp;
extern IPAddress ethernetIpv4;
extern IPAddress ethernetMasque;
extern IPAddress ethernetPasserelle;
extern bool wifiStatus;
extern bool wifiDhcp;
extern IPAddress wifiIpv4;
extern IPAddress wifiMasque;
extern IPAddress wifiPasserelle;
extern bool lteStatus;

// Variables globales Audio
extern float headphoneLevel;
extern unsigned int sampleRate;

#endif