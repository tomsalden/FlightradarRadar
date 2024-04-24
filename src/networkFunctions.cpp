#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>

#include "displayObject.h"
#include "parameterObject.h"
#include "defines.h"
#include "planesObject.h"

AsyncWebServer server(80);
int planeIcon = 0;

// Function to start the network connection with as parameters the ssid and password
void startNetworkConnection(const char* ssid, const char* password, DisplayObject * display){
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  int i = 0;
  long startTime = millis();

  while(WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > 500){
      if (i > 10){
        WiFi.reconnect();
        i = 0;
      }
      Serial.print(".");
      i ++;
      startTime = millis();
    }
    if (millis() - startTime > 100){
      display->updateSplashScreen("Welcome to FlightRadarRadar!\n Starting up");
    }
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  display->ipAddress = WiFi.localIP().toString();
  display->SSID = WiFi.SSID();
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

bool planeFilter(String planeCallsign, String planeModel, ParameterObject * filterParameters){
  //Filter planes based on if the model is in the importantPlaneModels array
  //First remove the " from the planeModel start and end, then check if it is in the importantPlaneModels array
  planeModel.remove(0,1);
  int tempLength = planeModel.length();
  planeModel.remove(tempLength-1,1);

  // size = 13;

  for (int i = 0; i < filterParameters->importantPlaneModelsSize; i++){
    if (planeModel == filterParameters->importantPlaneModels[i]){
      planeIcon = filterParameters->importantPlaneModelsIcons[i];
      return true;
    }
  }
  
  //Filter planes based on callsign
  //First remove the " from the planeCallsign start and end, then check if the importantCallsigns array contains a substring of the planeCallsign
  // int size = importantCallsigns[0].toInt(); //33;

  for (int i = 0; i < filterParameters->importantCallsignsSize; i++){
    // Serial.println(importantCallsigns[i] + " " + planeCallsign + " " + String(i));
    if (planeCallsign.indexOf(filterParameters->importantCallsigns[i]) != -1){
      return true;
    }
  }

  return false;
}


int networkRequestStream(float * locationSettings, PlanesObject * selectedPlanes, ParameterObject * requestParameters){
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
      planeIcon = 0;
      if (planeFilter(planeName, planeType, requestParameters) == true){
        Serial.println("Identifier: " + planeIdentifier);
        if (selectedPlanes->planeArrayFull == false){
          // Serial.println("Identifier: " + planeIdentifier + "Timestamp: " + planeTimestamp + " Plane: " + planeRegistration + " Type: " + planeType + " Name: " + planeName + " Lat: " + planeLat + " Lon: " + planeLon + " Alt: " + planeAltitude + " Heading: " + planeHeading + " Speed: " + planeSpeed);

          selectedPlanes->addPlaneInfo(planeIdentifier, planeName, planeLat, planeLon, planeAltitude, planeType, planeRegistration, planeHeading, planeSpeed, planeTimestamp, planeIcon);
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

void setupWebServer(ParameterObject * networkPreferences){
  // networkPreferences.begin("flightradar", false);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Redirect to the variables page
    request->redirect("/variables");
  });

    server.on("/variables", HTTP_GET, [networkPreferences](AsyncWebServerRequest *request) {
    String variablesPage = "<script>";
    variablesPage += "window.onload = function() {";
    variablesPage += "  if (sessionStorage.scrollPosition) {";
    variablesPage += "    window.scrollTo(0, sessionStorage.scrollPosition);";
    variablesPage += "  }";
    variablesPage += "};";
    variablesPage += "window.onbeforeunload = function() {";
    variablesPage += "  sessionStorage.scrollPosition = window.scrollY || document.documentElement.scrollTop;";
    variablesPage += "};";
    variablesPage += "</script>";
    variablesPage += "<html><body>";
    variablesPage += "<h1>Wifi information</h1>";
    variablesPage += "<p>SSID: " + networkPreferences->getString("ssid", "") + "</p>";
    variablesPage += "<p>Password: " + networkPreferences->getString("password", "") + "</p>";
    //Add form to change SSID
    variablesPage += "<h2>Change SSID</h2>";
    variablesPage += "<form action='/changewifi' method='post'>";
    variablesPage += "SSID: <input type='text' name='ssid'><br>";
    variablesPage += "<input type='submit' value='Change'></form>";
    variablesPage += "<p>Change will be applied after a reboot</p>";

    //Add form to change password
    variablesPage += "<h2>Change Password</h2>";
    variablesPage += "<form action='/changewifi' method='post'>";
    variablesPage += "Password: <input type='text' name='password'><br>";
    variablesPage += "<input type='submit' value='Change'></form>";
    variablesPage += "<p>Change will be applied after a reboot</p>";

    //Add form to change current latitute and longitude
    variablesPage += "<h1>Current location</h1>";
    variablesPage += "<p>Current latitude: " + String(networkPreferences->getFloat("myLat", 0),6) + "</p>";
    variablesPage += "<p>Current longitude: " + String(networkPreferences->getFloat("myLon", 0),6) + "</p>";
    variablesPage += "<h2>Change current location</h2>";
    variablesPage += "<form action='/changelocation' method='post'>";
    variablesPage += "Latitude: <input type='text' name='latitude'><br>";
    variablesPage += "<input type='submit' value='Change'></form>";

    variablesPage += "<form action='/changelocation' method='post'>";
    variablesPage += "Longitude: <input type='text' name='longitude'><br>";
    variablesPage += "<input type='submit' value='Change'></form>";

    //Add small map of the current location with a marker
    variablesPage += "<h2>Current location</h2>";
    variablesPage += "<iframe src='https://maps.google.com/maps?q=" + String(networkPreferences->getFloat("myLat", 0),6) + "," + String(networkPreferences->getFloat("myLon", 0),6) + "&z=15&output=embed' width='600' height='450' frameborder='0' style='border:0' allowfullscreen></iframe>";


    // Add section to add important callsigns
    variablesPage += "<h1>Callsigns</h1>";
    variablesPage += "<table border='1'><tr><th>Callsign</th><th>Action</th></tr>";

    // Retrieve the number of variables
    int numVariablesCallsign = networkPreferences->getInt("num_cs", 0);

    for (int i = 1; i <= numVariablesCallsign; i++) {
      // Retrieve variable value from EEPROM
      String variable = networkPreferences->getString(("cs-" + String(i)).c_str(), "");

      // Display variable in a table row with remove button
      variablesPage += "<tr><td>" + variable + "</td>";
      variablesPage += "<td><form action='/removecs' method='post'><input type='hidden' name='id' value='" + String(i) + "'><input type='submit' value='Remove'></form></td></tr>";
    }

    variablesPage += "</table>";

    // Add form to add new variable
    variablesPage += "<h2>Add Callsign</h2>";
    variablesPage += "<form action='/addcs' method='post'>";
    variablesPage += "(Part of) callsign: <input type='text' name='variable'><br>";
    variablesPage += "<input type='submit' value='Add'></form>";


    //Add section to add important plane models
    variablesPage += "<h1>Plane models</h1>";
    variablesPage += "<table border='1'><tr><th>Plane model</th><th>Image</th><th>Action</th></tr>";

    // Retrieve the number of variables
    int numVariablesModels = networkPreferences->getInt("num_pm", 0);

    for (int i = 1; i <= numVariablesModels; i++) {
      // Retrieve variable value from EEPROM
      String variable = networkPreferences->getString(("pm-" + String(i)).c_str(), "");

      // Display variable in a table row with a dropdown menu to choose the image number to show and a remove button
      variablesPage += "<tr><td>" + variable + "</td>";
      // Add a dropdown menu to choose the image number to show
      int currentValue = networkPreferences->getInt(("pm-" + String(i) + "-img").c_str(), 0); // Replace 'yourIntegerVariable' with your actual variable

      variablesPage += "<td><form action='/changeimage' method='post'><input type='hidden' name='id' value='" + String(i) + "'><select name='image'>";

      for (int j = 0; j <= 8; j++) {
        variablesPage += "<option value='" + String(j) + "'";
        if (j == currentValue) {
          variablesPage += " selected";
        }
        variablesPage += ">" + String(j) + "</option>";
      }

      variablesPage += "</select><input type='submit' value='Change'></form></td>";  
      
      variablesPage += "<td><form action='/removepm' method='post'><input type='hidden' name='id' value='" + String(i) + "'><input type='submit' value='Remove'></form></td></tr>";
    }

    variablesPage += "</table>";

    // Add form to add new variable
    variablesPage += "<h2>Add Plane model</h2>";
    variablesPage += "<form action='/addpm' method='post'>";
    variablesPage += "Plane model: <input type='text' name='variable'><br>";
    variablesPage += "<input type='submit' value='Add'></form>";


    variablesPage += "</body></html>";

    request->send(200, "text/html", variablesPage);
  });

  server.on("/addcs", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable from form data
    if (request->hasParam("variable", true)) {
      AsyncWebParameter* p = request->getParam("variable", true);
      String variable = p->value();

      // Save variable to EEPROM
      int numVariables = networkPreferences->getInt("num_cs", 0);
      numVariables++;
      networkPreferences->putString(("cs-" + String(numVariables)).c_str(), variable);
      networkPreferences->putInt("num_cs", numVariables);
    }
    // Reload the parameters
    networkPreferences->reloadParameters();
    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.on("/addpm", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable from form data
    if (request->hasParam("variable", true)) {
      AsyncWebParameter* p = request->getParam("variable", true);
      String variable = p->value();

      // Save variable to EEPROM
      int numVariables = networkPreferences->getInt("num_pm", 0);
      numVariables++;
      networkPreferences->putString(("pm-" + String(numVariables)).c_str(), variable);
      networkPreferences->putInt("num_pm", numVariables);
    }

    // Reload the parameters
    networkPreferences->reloadParameters();

    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.on("/removecs", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable ID from form data
    if (request->hasParam("id", true)) {
      AsyncWebParameter* p = request->getParam("id", true);
      int id = p->value().toInt();

      // Remove variable from EEPROM
      networkPreferences->remove(("cs-" + String(id)).c_str());

      //Reorder the variables in EEPROM to prevent gaps
      for (int i = id; i <= networkPreferences->getInt("num_cs", 0)-1; i++) {
        String variable = networkPreferences->getString(("cs-" + String(i + 1)).c_str(), "");
        networkPreferences->putString(("cs-" + String(i)).c_str(), variable);
        networkPreferences->remove(("cs-" + String(i + 1)).c_str());
      }

      // Update number of variables
      int numVariables = networkPreferences->getInt("num_cs", 0);
      if (numVariables > 0) {
        numVariables--;
        networkPreferences->putInt("num_cs", numVariables);
      }
    }

    // Reload the parameters
    networkPreferences->reloadParameters();

    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.on("/removepm", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable ID from form data
    if (request->hasParam("id", true)) {
      AsyncWebParameter* p = request->getParam("id", true);
      int id = p->value().toInt();

      // Remove variable from EEPROM
      networkPreferences->remove(("pm-" + String(id)).c_str());
      networkPreferences->remove(("pm-" + String(id) + "-img").c_str());

      //Reorder the variables in EEPROM to prevent gaps
      for (int i = id; i <= networkPreferences->getInt("num_pm", 0)-1; i++) {
        String variable = networkPreferences->getString(("pm-" + String(i + 1)).c_str(), "");
        networkPreferences->putString(("pm-" + String(i)).c_str(), variable);
        networkPreferences->remove(("pm-" + String(i + 1)).c_str());

        int image = networkPreferences->getInt(("pm-" + String(i + 1) + "-img").c_str(), 0);
        networkPreferences->putInt(("pm-" + String(i) + "-img").c_str(), image);
        networkPreferences->remove(("pm-" + String(i + 1) + "-img").c_str());
      }

      // Update number of variables
      int numVariables = networkPreferences->getInt("num_pm", 0);
      if (numVariables > 0) {
        numVariables--;
        networkPreferences->putInt("num_pm", numVariables);
      }
    }

    // Reload the parameters
    networkPreferences->reloadParameters();

    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.on("/changewifi", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable from form data
    if (request->hasParam("ssid", true)) {
      AsyncWebParameter* p = request->getParam("ssid", true);
      String variable = p->value();

      // Save SSID to EEPROM
      Serial.println("Changing SSID to: " + variable);
      networkPreferences->putString("ssid", variable);
    }

    if (request->hasParam("password", true)) {
      AsyncWebParameter* p = request->getParam("password", true);
      String variable = p->value();

      // Save SSID to EEPROM
      Serial.println("Changing Password to: " + variable);
      networkPreferences->putString("password", variable);
    }

    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.on("/changelocation", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable from form data
    if (request->hasParam("latitude", true)) {
      AsyncWebParameter* p = request->getParam("latitude", true);
      String variable = p->value();

      // Save SSID to EEPROM
      networkPreferences->putFloat("myLat", variable.toFloat());
    }

    if (request->hasParam("longitude", true)) {
      AsyncWebParameter* p = request->getParam("longitude", true);
      String variable = p->value();

      // Save SSID to EEPROM
      networkPreferences->putFloat("myLon", variable.toFloat());
    }

    // Reload the parameters
    networkPreferences->reloadParameters();

    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.on("/changeimage", HTTP_POST, [networkPreferences](AsyncWebServerRequest *request) {
    // Retrieve variable ID and image number from form data
    if (request->hasParam("id", true)) {
      AsyncWebParameter* p = request->getParam("id", true);
      int id = p->value().toInt();

      if (request->hasParam("image", true)) {
        AsyncWebParameter* p = request->getParam("image", true);
        int image = p->value().toInt();

        // Save image number to EEPROM
        networkPreferences->putInt(("pm-" + String(id) + "-img").c_str(), image);

        Serial.println("Changing image for plane model " + String(id) + " to: " + String(image));
      }
    }

    // Reload the parameters
    networkPreferences->reloadParameters();

    // Redirect back to variables page
    request->redirect("/variables");
  });

  server.begin();
}

