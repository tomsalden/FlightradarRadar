#include <Arduino.h>

#include "variables.h"
#include "defines.h"
#include "planesObject.h"
#include "networkFunctions.h"
#include "displayObject.h"


TaskHandle_t TaskDisp;
TaskHandle_t TaskSens;
TaskHandle_t TaskFlight;

//Plane object
PlanesObject selectedPlanes;
PlanesObject displayPlanes;

//Display object
DisplayObject display;

float locationVariable[7] = {areaMaxLat, areaMinLat, areaMaxLon, areaMinLon, myLat, myLon, myAlt};

const char* ntpServer = "pool.ntp.org";
long offsetTime = 0; //Variable to keep track of millisecond time

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

//Handles the inputs and outputs of the system: compass, buttons, buzzer, battery etc.
void TaskSensCode( void * pvParameters ){
  Serial.print("TaskDisp running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    Serial.println("TaskSens!");
    delay(2000);
  } 
}

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
  startNetworkConnection(ssid, password);
  configTime(0, 0, ntpServer);
  Serial.println("Netwerk gedaan");

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