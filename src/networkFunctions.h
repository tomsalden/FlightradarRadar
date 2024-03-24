#ifndef networkFunctions_h
#define networkFunctions_h

void startNetworkConnection(const char* ssid, const char* password);
bool networkStatus();
unsigned long getTime();
unsigned long getTimeOffset();
// int networkRequest(float * locationSettings);
int networkRequestStream(float * locationSettings, PlanesObject * selectedPlanes, String importantCallsigns[], String importantPlaneModels[]);
bool planeFilter(String planeCallsign, String planeModel, String importantCallsigns[], String importantPlaneModels[]);

#endif