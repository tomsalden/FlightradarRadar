#ifndef variables_h
#define variables_h

//#############   Variables       #############

//Wifi settings
String ssid = "wifi_ssid";
String password = "wifi_password";


// Coordinates
// Search area settings
// float areaMaxLat = 85.5007; //Extremely big area, used for testing if memory errors occur
// float areaMinLat = 30.0000;
// float areaMaxLon = 57.6836;
// float areaMinLon = -10.6271;

float areaMaxLat = 54.2; //Netherlands
float areaMinLat = 50.0;
float areaMaxLon = 10.0;
float areaMinLon = 1.5;

// # Current coordinates (it's in the sea :D)
float myLat = 52.252429;
float myLon = 4.307893;
float myAlt = 0;

//Callsigns to look out for, the last item is the amount of callsigns in the array with the number of items in the array as the first item
String importantCallsigns[] = {"34", 
                                "RCH","LAGR","RRR","NCHO",
                                "MMF","NAF","NATO","RED",
                                "HKY","QID","CFC","JAKE",
                                "ASCOT","HOBO","BART","BLKCAT",
                                "OMEN","BLUE","BGA","ZXP",
                                "DGLBA","LIFE","ZXP","BAF",
                                "CHAOS","MOOSE","WOLF","SNAKE",
                                "CEF","NOBLE","ROGUE","SVF","TRA"
                                };

//Plane types to look out for, the last item is the amount of plane types in the array with the number of items in the array as the first item
String importantPlaneModels[] = {"13",
                                "C17","R135","A400","C30J",
                                "A124","EUFI","C130","H47",
                                "F35","K35R","HAWK","P8",
                                "C5M"};

//variable to check if parameters are updated
bool parametersUpdated = false;
#endif