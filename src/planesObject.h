#ifndef PLANES_OBJECT_H
#define PLANES_OBJECT_H

#include <Arduino.h>
#include "defines.h"

class PlanesObject {
public:
    // Functions for the planes object
    void init(float myLat, float myLon);
    void addPlaneInfo(String planeIdentifier, String planeName, String planeLat, String planeLon, String planeAltitude, String planeType, String planeRegistration, String planeHeading, String planeSpeed, String planeTimestamp);
    int calculateDistance(float myLat, float myLon, float planeLat, float planeLon);
    void sortPlanes();
    void clearPlanes();
    void printClosestPlane();

    // Variables for the planes object
    String planeIdentifier[planeArraySize];
    String planeName[planeArraySize];
    float planeLat[planeArraySize];
    float planeLon[planeArraySize];
    int planeAltitude[planeArraySize];
    String planeType[planeArraySize];
    String planeRegistration[planeArraySize];
    int planeHeading[planeArraySize];
    int planeSpeed[planeArraySize];
    long planeTimestamp[planeArraySize];
    int planeDistance[planeArraySize];
    int sortedPlaneArray[planeArraySize];

    // Variables for calculations
    float myLat;
    float myLon;
    float myAlt;
    int planeCount = 0;
    bool planeArrayFull = false;
    bool planeArrayLocked = false;
    bool parametersUpdated = false;
};

#endif // PLANES_OBJECT_H
