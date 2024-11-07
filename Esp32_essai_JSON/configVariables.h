// configVariables.h
#ifndef CONFIGVARIABLES_H
#define CONFIGVARIABLES_H

// Variables globales Network
extern const char* deviceName;
extern bool ethernetStatus;
extern bool ethernetDhcp;
extern const char* ethernetIpv4;
extern const char* ethernetMasque;
extern const char* ethernetPasserelle;
extern bool wifiStatus;
extern bool wifiDhcp;
extern const char* wifiIpv4;
extern const char* wifiMasque;
extern const char* wifiPasserelle;
extern bool lteStatus;

// Variables globales Audio
extern float headphoneLevel;
extern int samplingRate;

#endif