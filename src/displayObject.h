#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <Arduino.h>
#include "planesObject.h"
#include <TFT_eSPI.h>
#include <rm67162.h>

class DisplayObject {
public:
    // Functions for the display object
    void init();
    void updateSplashScreen(String splashText);
    void updateDisplay(PlanesObject * displayPlanes, int selectedPlaneIndex, long offsetTime);
    void turnOffDisplay();
    long splashScreenTime = 0;
    String ipAddress = "0.0.0.0";
    String SSID = "SSID";

private:
    // Functions for the display object
    void drawCompass(int angle);
    void drawIcon(String planeType);
    
    //Function to interpolate new latitude and longitude based on the current position, the heading, speed and the time passed since the last update
    float interpolateLat(float lat, float heading, float speed, float time);
    float interpolateLon(float lon, float heading, float speed, float time, float lat, float newLat);

    float calculateDistance(float myLat, float myLon, float planeLat, float planeLon);
    double calculateClosestDistance(float myLat, float myLon, float planeLat, float planeLon, int heading);
    double calculateTimeToClosestDistance(float planeLat, float planeLon, int planeSpeed);
    int calculateAngle(float myLat, float myLon, float planeLat, float planeLon);

    double latIntersect;
    double longIntersect;

    int timeSinceUpdate(long timestamp, long offsetTime);
    int anglePlane = 0;

    double previousTime = 0;
    double timeArray[5] = {0,0,0,0,0};
    int timeArrayIndex = 0;

    int splashScreenFrame = 0;
    String periods = "";

    int displayTimeout = 60000; //Time in milliseconds before the display will turn off
    

    // Variables for the display object
    TFT_eSPI tft = TFT_eSPI();

    TFT_eSprite background = TFT_eSprite(&tft);
    TFT_eSprite table = TFT_eSprite(&tft);
    TFT_eSprite planeCompassBase = TFT_eSprite(&tft);
    TFT_eSprite planeCompass = TFT_eSprite(&tft);
    TFT_eSprite radar = TFT_eSprite(&tft);
    TFT_eSprite radarStripe = TFT_eSprite(&tft);
    TFT_eSprite planeIcon = TFT_eSprite(&tft);
    TFT_eSprite splashScreenLogo = TFT_eSprite(&tft);
    TFT_eSprite splashScreenNeedle = TFT_eSprite(&tft);
    TFT_eSprite splashScreenNeedleHolder = TFT_eSprite(&tft);

    // uint32_t backgroundColour = 0x0065;
    uint32_t backgroundColour = TFT_BLACK;
    uint32_t splashScreenColour = 0x0253;

    uint32_t lineColour = 0xFFFF;
    uint32_t transparentColour = 0x632C;
    uint32_t headerTextColour = 0xFFE0;
    uint32_t textColour = 0x06FF;
    uint32_t radarColour = 0x0480;
};

#endif // DISPLAY_OBJECT_H