#include <WiFi.h>
#include <HTTPClient.h>

#include "defines.h"
#include "planesObject.h"


// Function to start the network connection with as parameters the ssid and password
void startNetworkConnection(const char* ssid, const char* password){
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  int i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    if (i > 10){
      WiFi.reconnect();
      i = 0;
    }
    delay(500);
    Serial.print(".");
    i ++;
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to check the network status. If it is not connected, retry the connection by using reconnect
bool networkStatus(){
  if(WiFi.status() != WL_CONNECTED){
    WiFi.reconnect();
    return false;
  }
  return true;
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(false);
  }
  time(&now);
  return now;
}

// Function that gets current epoch time in milliseconds related to Millis()
unsigned long getTimeOffset() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(false);
  }
  time(&now);
  long unixTime = now % 1000000;
  unixTime = unixTime * 1000;
  return unixTime - millis();
}

bool planeFilter(String planeCallsign, String planeModel, String importantCallsigns[], String importantPlaneModels[]){
  //Filter planes based on callsign
  //First remove the " from the planeCallsign start and end, then check if the importantCallsigns array contains a substring of the planeCallsign
  int size = importantCallsigns[0].toInt(); //33;

  for (int i = 1; i < size; i++){
    // Serial.println(importantCallsigns[i] + " " + planeCallsign + " " + String(i));
    if (planeCallsign.indexOf(importantCallsigns[i]) != -1){
      
      return true;
    }
  }


  //Filter planes based on if the model is in the importantPlaneModels array
  //First remove the " from the planeModel start and end, then check if it is in the importantPlaneModels array
  planeModel.remove(0,1);
  int tempLength = planeModel.length();
  planeModel.remove(tempLength-1,1);

  size = 13;

  for (int i = 1; i < size; i++){
    if (planeModel == importantPlaneModels[i]){
      return true;
    }
  }
  return false;
}


int networkRequestStream(float * locationSettings, PlanesObject * selectedPlanes, String importantCallsigns[], String importantPlaneModels[]){
  String url = "https://data-cloud.flightradar24.com/zones/fcgi/feed.js?bounds=" + String(locationSettings[0],4) + "," + String(locationSettings[1],4) + "," + String(locationSettings[3],4) + "," + String(locationSettings[2],4) + "&faa=1&satellite=1&mlat=1&flarm=1&adsb=1&gnd=0&air=1&vehicles=0&estimated=1&maxage=14400&gliders=0&stats=0";
  WiFiClient client;
  HTTPClient http;
  
  //Variables to test datastream
  //"33e7efbd":     [ "507C55"  , 50.39     , 52.89     , 307           , 42575          , 398         , ""    , "F-UWWW4"  , "B788"     , "UK78705"          , 1707325658       , "TAS"  , "DME"  ,"HY601" , 0      , -832   ,"UZB601"     , 0      , "UZB"]
  // planeIdentifier[ 0|temp    , 1|planeLat, 2|planeLon, 3|planeHeading, 4|planeAltitude, 5|planeSpeed, 6|temp, 7|temp     , 8|planeType, 9|planeRegistration, 10|planeTimestamp, 11|temp, 12|temp, 13|temp, 14|temp, 15|temp, 16|planeName, 17|temp, 18|temp  

  String planeIdentifier;
  String planeName;
  String planeLat;
  String planeLon;
  String planeAltitude;
  String planeType;
  String planeRegistration;
  String planeHeading;
  String planeSpeed;
  String planeTimestamp;
  String temp;

  //Check if planearray is not locked
  if (selectedPlanes->planeArrayLocked == true){
    return 0;
  }

  //send request
  http.useHTTP10(true);
  http.begin(url.c_str());

  //Get response
  int httpResponseCode = http.GET();

  if (httpResponseCode != 200){
    http.end();
    Serial.println("Error code: " + String(httpResponseCode));
    return httpResponseCode;
  }

  //Lock the plane array
  selectedPlanes->planeArrayLocked = true;
  selectedPlanes->clearPlanes();

  //parse Response
  Stream* dataStream = http.getStreamPtr();

  dataStream->find("\"version\":4,"); //Skip first part of data
  do { //Loop over whole document
    
    do{//Loop per stream
      planeIdentifier   = dataStream->readStringUntil('['); //Get the plane identifier
      temp              = dataStream->readStringUntil(','); //variable 0 = unnecessary
      planeLat          = dataStream->readStringUntil(','); //variable 1 = latitude
      planeLon          = dataStream->readStringUntil(','); //variable 2 = longitude
      planeHeading      = dataStream->readStringUntil(','); //variable 3 = heading
      planeAltitude     = dataStream->readStringUntil(','); //variable 4 = altitude
      planeSpeed        = dataStream->readStringUntil(','); //variable 5 = speed
      temp              = dataStream->readStringUntil(','); //variable 6 = unnecessary
      temp              = dataStream->readStringUntil(','); //variable 7 = unnecessary
      planeType         = dataStream->readStringUntil(','); //variable 8 = type
      planeRegistration = dataStream->readStringUntil(','); //variable 9 = registration/callsign
      planeTimestamp    = dataStream->readStringUntil(','); //variable 10 = timestamp
      temp              = dataStream->readStringUntil(','); //variable 11 = unnecessary
      temp              = dataStream->readStringUntil(','); //variable 12 = unnecessary
      temp              = dataStream->readStringUntil(','); //variable 13 = unnecessary
      temp              = dataStream->readStringUntil(','); //variable 14 = unnecessary
      temp              = dataStream->readStringUntil(','); //variable 15 = unnecessary
      planeName         = dataStream->readStringUntil(','); //variable 16 = name
      temp              = dataStream->readStringUntil(','); //variable 17 = unnecessary


      // test = dataStream->readString();
      // if (planeType == "\"B763\""){
      if (planeFilter(planeName, planeType, importantCallsigns, importantPlaneModels) == true){
        Serial.println("Identifier: " + planeIdentifier);
        if (selectedPlanes->planeArrayFull == false){
          // Serial.println("Identifier: " + planeIdentifier + "Timestamp: " + planeTimestamp + " Plane: " + planeRegistration + " Type: " + planeType + " Name: " + planeName + " Lat: " + planeLat + " Lon: " + planeLon + " Alt: " + planeAltitude + " Heading: " + planeHeading + " Speed: " + planeSpeed);

          selectedPlanes->addPlaneInfo(planeIdentifier, planeName, planeLat, planeLon, planeAltitude, planeType, planeRegistration, planeHeading, planeSpeed, planeTimestamp);
        }
      }
      

    } while (dataStream->findUntil(",","]"));
  } while (dataStream->findUntil(",","}"));
  
  //disconnect and clear up
  http.end();
  // delete dataStream;

  // Sort the planes
  selectedPlanes->sortPlanes();
  selectedPlanes->planeArrayLocked = false;

  return httpResponseCode;
}