#include <Arduino.h>
#include "Preferences.h"

#include "variables.h"
#include "defines.h"
#include "planesObject.h"
#include "networkFunctions.h"
#include "displayObject.h"

//Task handles
TaskHandle_t TaskDisp;
TaskHandle_t TaskSens;
TaskHandle_t TaskFlight;

//Plane object
PlanesObject selectedPlanes;
PlanesObject displayPlanes;

//Display object
DisplayObject display;

//Setup the preferences object
Preferences preferences;

float locationVariable[7] = {areaMaxLat, areaMinLat, areaMaxLon, areaMinLon, myLat, myLon, myAlt};

const char* ntpServer = "pool.ntp.org";
long offsetTime = 0; //Variable to keep track of millisecond time

//--------------------DISPLAY TASK--------------------
// Task function for the display

//Display task function. Handles current plane locatiton and handles the display
void TaskDispCode( void * pvParameters ){
  Serial.print("TaskDisp running on core ");
  Serial.println(xPortGetCoreID());


  for(;;){
    // Serial.println("TaskDisp!");
    display.updateDisplay(&displayPlanes, 0, offsetTime);
    delay(100);
  } 
}

//--------------------SENSORS TASK--------------------
// Task function for the sensors and for the webserver

//Handles the inputs and outputs of the system: compass, buttons, buzzer, battery etc.
//In addition handles the webserver that can be used to change the settings of the system
void TaskSensCode( void * pvParameters ){
  Serial.print("TaskDisp running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    Serial.println("TaskSens!");
    delay(2000);
  } 
}

//--------------------FLIGHT TASK--------------------
// Task function for the flight

//Flight task function. Handles the network and retieval of the flight information
void TaskFlightCode( void * pvParameters ){
  Serial.print("TaskFlight running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    Serial.println("TaskFlight!");
    delay(2000);
    if (not networkStatus()){ //Check if network is still connected. Otherwise reconnect automatically
      continue;
    }
    Serial.println(getTime());

    // Request plane data and store it in the selectedPlanes object
    Serial.println(networkRequestStream(&locationVariable[0], &selectedPlanes, importantCallsigns, importantPlaneModels));
    selectedPlanes.printClosestPlane();

    // Update the displayPlanes object with the selectedPlanes object
    while(displayPlanes.planeArrayLocked == true){
      delay(100);
    }
    if (displayPlanes.planeArrayLocked == false){
      displayPlanes.planeArrayLocked = true;
      displayPlanes = selectedPlanes;
      displayPlanes.planeArrayLocked = false;
    }

    offsetTime = getTimeOffset(); //Update the time offset
    

  } 
}


void setup() {
  Serial.begin(115200);
  Serial.println("Start");

  //Load the preferences and check if preferences already are present
  preferences.begin("flightradar", false);

  if (preferences.getBool("Saved", false) == false){ //If no preferences are saved, save the default preferences
    preferences.putFloat("areaMaxLat", areaMaxLat);
    preferences.putFloat("areaMinLat", areaMinLat);
    preferences.putFloat("areaMaxLon", areaMaxLon);
    preferences.putFloat("areaMinLon", areaMinLon);
    preferences.putFloat("myLat", myLat);
    preferences.putFloat("myLon", myLon);
    preferences.putFloat("myAlt", myAlt);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putBool("Saved", true);

  } else { //Preferences have been saved, load them
    areaMaxLat = preferences.getFloat("areaMaxLat", areaMaxLat);
    areaMinLat = preferences.getFloat("areaMinLat", areaMinLat);
    areaMaxLon = preferences.getFloat("areaMaxLon", areaMaxLon);
    areaMinLon = preferences.getFloat("areaMinLon", areaMinLon);
    myLat = preferences.getFloat("myLat", myLat);
    myLon = preferences.getFloat("myLon", myLon);
    myAlt = preferences.getFloat("myAlt", myAlt);
    ssid = preferences.getString("ssid", ssid);
    password = preferences.getString("password", password);
  }

  preferences.end();

  //
  startNetworkConnection(ssid.c_str(), password.c_str());
  configTime(0, 0, ntpServer);
  Serial.println("Netwerk gedaan");

  //Start webserver
  setupWebServer();

  //Initialize the display and the planes
  selectedPlanes.init(myLat, myLon);
  displayPlanes.init(myLat, myLon);
  display.init();

  //create a task that will be executed in the TaskDisp() function, with priority 1 and executed on core 0
  //Will manage everything related to the display (including calculating the interpolated data of the planes)
  xTaskCreatePinnedToCore(
                    TaskDispCode,   /* Task function. */
                    "Display",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskDisp,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the TaskSens() function, with priority 1 and executed on core 0
  //Will take compass info and battery info and outputs buzzer sounds
  xTaskCreatePinnedToCore(
                    TaskSensCode,   /* Task function. */
                    "Sensors",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskSens,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the TaskFlight() function, with priority 1 and executed on core 1
  //Will retrieve the filghts and process the information
  xTaskCreatePinnedToCore(
                    TaskFlightCode,   /* Task function. */
                    "FlightRetriever",     /* name of task. */
                    100000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskFlight,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
}

void loop() {
  
}