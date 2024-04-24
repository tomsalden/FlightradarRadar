#ifndef networkFunctions_h
#define networkFunctions_h

#include "DisplayObject.h" // Add the missing import statement for the 'DisplayObject' class

void startNetworkConnection(const char* ssid, const char* password,  DisplayObject * display);
bool networkStatus();
unsigned long getTime();
unsigned long getTimeOffset();
// int networkRequest(float * locationSettings);
int networkRequestStream(float * locationSettings, PlanesObject * selectedPlanes, ParameterObject * requestParameters);
bool planeFilter(String planeCallsign, String planeModel, ParameterObject * filterParameters);

void setupWebServer(ParameterObject * networkPreferences);
String base64_encode(const uint8_t* data, size_t length);

#endif