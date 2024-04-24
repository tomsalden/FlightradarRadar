#include <Arduino.h>
#include "Preferences.h"

#include "parameterObject.h"
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

//Setup the parameters object
ParameterObject parameters;

//Display variables
bool splashScreen = true;
String splashText = "Welcome to FlightRadarRadar!\n Starting up";

float locationVariable[7] = {parameters.areaMaxLat, parameters.areaMinLat, parameters.areaMaxLon, parameters.areaMinLon, parameters.myLat, parameters.myLon, parameters.myAlt};
int selectedPlane = 0;

int button1Pin = 0;
int button2Pin = 21;

int lastButtonState1 = HIGH; // the previous reading from the input pin
int lastButtonState2 = HIGH; // the previous reading from the input pin
unsigned long lastDebounceTime1 = 0; // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers

const char* ntpServer = "pool.ntp.org";
long offsetTime = 0; //Variable to keep track of millisecond time

//--------------------DISPLAY TASK--------------------
// Task function for the display

//Display task function. Handles current plane locatiton and handles the display
void TaskDispCode( void * pvParameters ){
  Serial.print("TaskDisp running on core ");
  Serial.println(xPortGetCoreID());


  for(;;){
    if (splashScreen == true){
      display.updateSplashScreen(splashText);
      delay(100);
      continue;
    }
    if (selectedPlane >= displayPlanes.planeCount){
      selectedPlane = 0;
    }
    display.updateDisplay(&displayPlanes, selectedPlane, offsetTime);
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
    // Serial.println("TaskSens!");
    //Read button 1
    int reading1 = digitalRead(button1Pin);

    if (reading1 != lastButtonState1) {
      Serial.println("Button1 pressed Noticed");
      lastDebounceTime1 = millis();
      selectedPlane = (selectedPlane + 1) % displayPlanes.planeCount;
    }

    if ((millis() - lastDebounceTime1) > debounceDelay) {
      if (reading1 != lastButtonState1) {
        lastButtonState1 = reading1;

        if (reading1 == HIGH) {
          Serial.println("Button1 pressed");

        }
      }
    }

    //Read button 2
    int reading2 = digitalRead(button2Pin);
    
    if (reading2 != lastButtonState2) {
      Serial.println("Button2 pressed Noticed");
      lastDebounceTime2 = millis();
      selectedPlane = (selectedPlane - 1) % displayPlanes.planeCount;
      if (selectedPlane < 0){
        selectedPlane = displayPlanes.planeCount - 1;
      }
    }

    if ((millis() - lastDebounceTime2) > debounceDelay) {
      if (reading2 != lastButtonState2) {
        lastButtonState2 = reading2;

        if (reading2 == HIGH) {
          Serial.println("Button2 pressed");
        }
      }
    }

    delay(100);
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
      if (splashScreen == false){
        splashScreen = true;
        display.splashScreenTime = millis();
        splashText = "Network disconnected!\n Reconnecting";
      }
      continue;
    }
    Serial.println(getTime());

    // Request plane data and store it in the selectedPlanes object
    locationVariable[0] = parameters.areaMaxLat;
    locationVariable[1] = parameters.areaMinLat;
    locationVariable[2] = parameters.areaMaxLon;
    locationVariable[3] = parameters.areaMinLon;
    locationVariable[4] = parameters.myLat;
    locationVariable[5] = parameters.myLon;
    locationVariable[6] = parameters.myAlt;

    Serial.println(networkRequestStream(&locationVariable[0], &selectedPlanes, &parameters));
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

    if (displayPlanes.planeCount > 0){
      splashScreen = false;
    }
    else{
      if (splashScreen == false){
        splashScreen = true;
        display.splashScreenTime = millis();
        splashText = "No planes found in the area.\n Searching planes";
      }
    }

    offsetTime = getTimeOffset(); //Update the time offset
    

  } 
}


void setup() {
  // Initialize display and show the splash screen
  display.init();
  display.updateSplashScreen(splashText);
  splashScreen = true;
  display.splashScreenTime = millis();


  //Initialize serial communication
  Serial.begin(115200);
  Serial.println("Start");

  delay(3000);

  parameters.init(); //Initialize the parameters object

  startNetworkConnection(parameters.ssid.c_str(), parameters.password.c_str(), &display);
  configTime(0, 0, ntpServer);
  Serial.println("Netwerk gedaan");
  splashText = "Network connected!\n Retrieving flights";

  //Start webserver
  setupWebServer(&parameters);

  //Initialize the display and the planes
  selectedPlanes.init(parameters.myLat, parameters.myLon);
  displayPlanes.init(parameters.myLat, parameters.myLon);

  //Setup button pins
  // pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);

  //create a task that will be executed in the TaskDisp() function, with priority 1 and executed on core 0
  //Will manage everything related to the display (including calculating the interpolated data of the planes)
  xTaskCreatePinnedToCore(
                    TaskDispCode,   /* Task function. */
                    "Display",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    configMAX_PRIORITIES,           /* priority of the task */
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