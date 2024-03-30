#include "displayObject.h"
#include "NotoSansBold36.h"
#include "Latin_Hiragana_24.h"
#include "PlaneCompass.h"
#include "plane1.h"
#include "plane2.h"
#include "plane3.h"
#include "plane4.h"
#include "plane5.h"
#include "plane6.h"

// Functions for the display object
void DisplayObject::init() {
    // Initialize the display object
    // Set initial values for the display
    Serial.println("Initializing display object");
    rm67162_init();
    lcd_setRotation(3);

    tft.setSwapBytes(true);

    DisplayObject::background.createSprite(536,240);
    table.createSprite(536,240);
    planeCompassBase.createSprite(70,70);
    planeCompass.createSprite(70,70);
    radar.createSprite(70,70);
    radarStripe.createSprite(4,70);
    planeIcon.createSprite(88,88);
    planeIcon.setSwapBytes(true);

    planeIcon.pushImage(0,0,88,88,Plane6);

    planeCompass.pushImage(0,0,70,70,PlaneCompass);
    planeCompass.setPivot(35,35);
}

void DisplayObject::updateDisplay(PlanesObject * displayPlanes, int selectedPlaneIndex, long offsetTime) {
    // Interpolate plane information
    int selectedPlane = displayPlanes->sortedPlaneArray[selectedPlaneIndex];
    int timeDifference = timeSinceUpdate(displayPlanes->planeTimestamp[selectedPlane], offsetTime);
    float newLat = interpolateLat(displayPlanes->planeLat[selectedPlane], displayPlanes->planeHeading[selectedPlane], displayPlanes->planeSpeed[selectedPlane], timeDifference);
    float newLon = interpolateLon(displayPlanes->planeLon[selectedPlane], displayPlanes->planeHeading[selectedPlane], displayPlanes->planeSpeed[selectedPlane], timeDifference, displayPlanes->planeLat[selectedPlane], newLat);
    float newDistance = calculateDistance(displayPlanes->myLat, displayPlanes->myLon, newLat, newLon);
    double closestDistance = calculateClosestDistance(displayPlanes->myLat, displayPlanes->myLon, newLat, newLon, displayPlanes->planeHeading[selectedPlane]);
    double timeToClosestDistance = calculateTimeToClosestDistance(newLat, newLon, displayPlanes->planeSpeed[selectedPlane]);
    int planeHeading = displayPlanes->planeHeading[selectedPlane];
    
    int minutesToClosestDistance = int(timeToClosestDistance/60);
    int secondsToClosestDistance = int(timeToClosestDistance) % 60;

    anglePlane = calculateAngle(displayPlanes->myLat, displayPlanes->myLon, newLat, newLon);
    
    if (displayPlanes->planeDistance[selectedPlane] == 99999){
        newDistance = 0;
        closestDistance = 0;
        minutesToClosestDistance = 0;
        secondsToClosestDistance = 0;
    }


    // Update the display with the current information
    DisplayObject::background.fillSprite(backgroundColour);
    table.fillSprite(transparentColour);
    radar.fillSprite(transparentColour);
    // DisplayObject::background.pushSprite(0,0);

    // Draw the lines for the table
    for (int i = 0; i < 1; i++){
        // Horizontal lines 1st column
        table.drawLine(0,51+i,430,51+i,lineColour);
        table.drawLine(0,51+1,430,51+1,lineColour);
        table.drawLine(0,89+i,236,89+i,lineColour);
        table.drawLine(0,127+i,236,127+i,lineColour);
        table.drawLine(0,165+i,236,165+i,lineColour);
        table.drawLine(0,203+i,430,203+i,lineColour);
        
        // Vertical line 1st column
        table.drawLine(236+i, 51, 236+i, 203, lineColour);

        // Horizontal lines 2nd column
        table.drawLine(236, 101+i, 430, 101+i, lineColour);
        table.drawLine(236, 151+i, 430, 151+i, lineColour);

        // Vertical line 2nd column
        table.drawLine(430+i, 0, 430+i, 240, lineColour);

        // Vertical line compass and planecounter
        table.drawLine(270+i, 0, 270+i, 51, lineColour);
        table.drawLine(350+i, 0, 350+i, 51, lineColour);

        // Horizontal lines 3rd column
        table.drawLine(430, 80+i, 536, 80+i, lineColour);  
        table.drawLine(430, 160+i, 536, 160+i, lineColour);   
    }

    // Plane name on top of screen
    table.loadFont(NotoSansBold36);
    table.setTextColor(headerTextColour);
    table.setTextDatum(1);
    table.drawString(displayPlanes->planeName[selectedPlane], 135, 9);
    table.unloadFont();

    // Plane information
    table.setTextColor(textColour);
    table.loadFont(Latin_Hiragana_24);
    table.setTextDatum(0);
    table.drawString("Type: ", 8, 59);
    table.drawString(displayPlanes->planeType[selectedPlane], 86, 59);
    table.drawString("Reg: ", 13, 100);
    table.drawString(displayPlanes->planeRegistration[selectedPlane], 86, 100);
    table.drawString("Alt: ", 26, 137);
    table.drawString(String(displayPlanes->planeAltitude[selectedPlane]), 86, 137);
    table.drawString("Spd: ", 13, 174);
    table.drawString(String(displayPlanes->planeSpeed[selectedPlane]), 86, 174);
    table.drawString(displayPlanes->planeName[selectedPlane], 12, 212);

    // Text column 2
    table.drawString("Dist: ", 270, 68);
    table.drawString(String(newDistance,1), 330, 68);

    table.drawString("C.Dist: ", 248, 118);
    table.drawString(String(closestDistance,1), 330, 118);

    table.drawString("T.Dist: ", 248, 168);
    table.drawString(String(minutesToClosestDistance) + "m, " + String(secondsToClosestDistance) + " s", 330, 168);

    // Unload font
    table.unloadFont();

    // Draw the planeradar
    int center = 35;
    radar.drawSmoothArc(center,center,28,25,0,360,radarColour,transparentColour,true);
    radar.drawSmoothArc(center,center,13,12,0,360,radarColour,transparentColour,true);
    radar.drawSmoothArc(center,center,2,0,0,360,radarColour,transparentColour,true);

    radar.drawWideLine(center,center+20,center,center+32,3,radarColour,transparentColour);
    radar.drawWideLine(center,center-20,center,center-32,3,radarColour,transparentColour);
    radar.drawWideLine(center-20,center,center-32,center,3,radarColour,transparentColour);
    radar.drawWideLine(center+20,center,center+32,center,3,radarColour,transparentColour);

    // Draw the radar stripe and rotate it 
    radarStripe.fillSprite(transparentColour);
    radarStripe.drawWideLine(2,center,2,center+33,4,0xFFFF,transparentColour);
    radarStripe.pushRotated(&radar, (anglePlane+180)%360, transparentColour);

    //Push the planeCompass to the base
    planeCompass.pushRotated(&planeCompassBase, planeHeading, transparentColour);

    // Push sprites to background
    table.pushToSprite(&background,0,0,transparentColour);
    planeCompassBase.pushToSprite(&background,448,7,transparentColour);
    radar.pushToSprite(&background,448,85,transparentColour);
    planeIcon.pushToSprite(&background,450,161);
    // radarStripe.pushToSprite(&background,481,120,transparentColour);

    // Push the LCD colours
    lcd_PushColors(0, 0, 536, 240, (uint16_t*)DisplayObject::background.getPointer());

}

//Update latitude
float DisplayObject::interpolateLat(float lat, float heading, float speed, float time){
    // Interpolate the latitude based on the current position, the heading, speed (km/h) and the time passed since the last update (ms)
    
    float distance = speed * time / 3600;
    float radLat = lat * PI / 180;
    float radHeading = heading * PI / 180;
    float newLat = asin(sin(radLat) * cos(distance / 6371e3) + cos(radLat) * sin(distance / 6371e3) * cos(radHeading));
    newLat = newLat * 180 / PI;
    return newLat;
}

//Update longitude
float DisplayObject::interpolateLon(float lon, float heading, float speed, float time, float lat, float newLat){
    // Interpolate the longitude based on the current position, the heading, speed (km/h) and the time passed since the last update (s)
    
    float distance = speed * time / 3600;
    float radLat = lat * PI / 180;
    float radLon = lon * PI / 180;
    float newLatRad = newLat * PI / 180;
    float radHeading = heading * PI / 180;
    float newLon = radLon + atan2(sin(radHeading) * sin(distance / 6371e3) * cos(radLat), cos(distance / 6371e3) - sin(radLat) * sin(newLatRad));
    newLon = newLon * 180 / PI;
    return newLon;
}

float DisplayObject::calculateDistance(float myLat, float myLon, float planeLat, float planeLon) {
    // Calculate the distance between two coordinates
    // Return the distance in kilometers

    // Convert the coordinates to radians
    float lat1 = myLat * PI / 180;
    float lat2 = planeLat * PI / 180;
    float lon1 = myLon * PI / 180;
    float lon2 = planeLon * PI / 180;

    // Calculate the distance
    float a = sin((lat2-lat1)/2)*sin((lat2-lat1)/2) + cos(lat1)*cos(lat2)*sin((lon2-lon1)/2)*sin((lon2-lon1)/2);
    float c = 2*atan2(sqrt(a), sqrt(1-a));
    return 6371 * c;

    return sqrt(pow(abs(planeLat - myLat), 2) + pow(abs(planeLon - myLon), 2));
}

// Function that gets the time since the last update in seconds
int DisplayObject::timeSinceUpdate(long timestamp, long offsetTime) {
  timestamp = timestamp % 1000000;
  timestamp = timestamp * 1000;
  return (offsetTime + millis()) - timestamp; //Time difference in ms
}

double DisplayObject::calculateClosestDistance(float myLat, float myLon, float planeLat, float planeLon, int heading) {
    // Calculate the closest distance the plane will be to the user
    // Return the closest distance in kilometers
    double headingRad = (360 - heading + 90) * PI / 180;

    //calculate the slope and y-intercept
    double aPlane = tan(headingRad);                //Slope
    double bPlane = planeLat - aPlane * planeLon;   //Y-intercept

    //calculate the slope and y-intercept for the line perpendicular to the plane's heading
    double aMyLoc = -1 / aPlane;                         //Slope
    double bMyLoc = myLat - aMyLoc * myLon;              //Y-intercept

    //calculate the intersection point
    longIntersect = (bMyLoc - bPlane) / (aPlane - aMyLoc);
    latIntersect = aPlane * longIntersect + bPlane;

    // Serial.println("Intersection point: " + String(latIntersect) + ", " + String(longIntersect));

    return calculateDistance(myLat, myLon, latIntersect, longIntersect);

    // //convert the points to km
    // double latIntersectKm = latIntersect * 110.574;
    // double longIntersectKm = longIntersect * 111.320 * cos(latIntersect * PI / 180);

    // double myLatKm = myLat * 110.574;
    // double myLonKm = myLon * 111.320 * cos(myLat * PI / 180);

    // //calculate the distance between the two points
    // double distance = sqrt(pow(abs(latIntersectKm - myLatKm), 2) + pow(abs(longIntersectKm - myLonKm), 2));
    // return distance;
}

double DisplayObject::calculateTimeToClosestDistance(float planeLat, float planeLon, int planeSpeed) {
    // Calculate the time it will take for the plane to reach the closest distance to the user
    // Return the time in seconds

    double distance = calculateDistance(planeLat, planeLon, latIntersect, longIntersect);

    // double distance = sqrt(pow(abs(latIntersect - planeLat), 2) + pow(abs(longIntersect - planeLon), 2));
    // Serial.println(String(latIntersect) + " " + String(planeLat) + " " + String(longIntersect) + " " + String(planeLon));
    return distance / planeSpeed * 3600;
    // return distance / (planeSpeed / 3600);
}

int DisplayObject::calculateAngle(float myLat, float myLon, float planeLat, float planeLon) {
    // Calculate the angle between the user and the plane
    // Return the angle in degrees

    // Convert the coordinates to radians
    myLat = myLat * PI / 180;
    myLon = myLon * PI / 180;
    planeLat = planeLat * PI / 180;
    planeLon = planeLon * PI / 180;

    // Calculate the angle
    float x = cos(planeLat) * sin(planeLon - myLon);
    float y = cos(myLat) * sin(planeLat) - sin(myLat) * cos(planeLat) * cos(planeLon - myLon);
    float angle = atan2(x,y) * 180 / PI;
    return int(angle);
}