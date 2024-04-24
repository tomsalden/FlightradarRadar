#include "planesObject.h"

void PlanesObject::init(float myLat, float myLon) {
    // Initialize the planes object
    // Set initial values for the plane arrays
    PlanesObject::myLat = myLat;
    PlanesObject::myLon = myLon;
    PlanesObject::planeCount = 0;
}

void PlanesObject::addPlaneInfo(String planeIdentifier, String planeName, String planeLat, String planeLon, String planeAltitude, String planeType, String planeRegistration, String planeHeading, String planeSpeed, String planeTimestamp, int planeIcon) {
    // Add plane information to the respective arrays
    int tempLength;
    // Serial.println("Addng plane info in count: " + PlanesObject::planeCount);

    // planeIdentifier
    planeIdentifier.remove(0,1);
    tempLength = planeIdentifier.length();
    planeIdentifier.remove(tempLength-1,2);
    PlanesObject::planeIdentifier[PlanesObject::planeCount] = planeIdentifier;

    // planeName
    planeName.remove(0,1);
    tempLength = planeName.length();
    planeName.remove(tempLength-1,1);
    PlanesObject::planeName[PlanesObject::planeCount] = planeName;

    // planeLat
    PlanesObject::planeLat[PlanesObject::planeCount] = planeLat.toFloat();

    // planeLon
    PlanesObject::planeLon[PlanesObject::planeCount] = planeLon.toFloat();

    // planeAltitude (Convert from feet to meters)
    PlanesObject::planeAltitude[PlanesObject::planeCount] = planeAltitude.toInt() * 0.3048;

    // planeType
    planeType.remove(0,1);
    tempLength = planeType.length();
    planeType.remove(tempLength-1,1);
    PlanesObject::planeType[PlanesObject::planeCount] = planeType;

    // planeRegistration
    planeRegistration.remove(0,1);
    tempLength = planeRegistration.length();
    planeRegistration.remove(tempLength-1,1);
    PlanesObject::planeRegistration[PlanesObject::planeCount] = planeRegistration;

    // planeHeading
    PlanesObject::planeHeading[PlanesObject::planeCount] = planeHeading.toInt();

    // planeSpeed (Convert from knt to km/h)
    PlanesObject::planeSpeed[PlanesObject::planeCount] = planeSpeed.toInt() * 1.852;

    // planeTimestamp
    PlanesObject::planeTimestamp[PlanesObject::planeCount] = planeTimestamp.toInt();

    // Calculate the distance between the plane and the user
    PlanesObject::planeDistance[PlanesObject::planeCount] = PlanesObject::calculateDistance(PlanesObject::myLat, PlanesObject::myLon, planeLat.toFloat(), planeLon.toFloat());

    // planeIcon
    PlanesObject::planeIcon[PlanesObject::planeCount] = planeIcon;

    // Increase the plane count
    PlanesObject::planeCount++;

    // Check if the plane array is full
    if (PlanesObject::planeCount == planeArraySize) {
        PlanesObject::planeArrayFull = true;
    }
}

int PlanesObject::calculateDistance(float myLat, float myLon, float planeLat, float planeLon) {
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

void PlanesObject::sortPlanes() {
    // Save the correct order of the planes based on distance in the sortedPlaneArray
    // Update the sortedPlaneArray based on the altitude angle from the user
    
    //Convert my coordinates to radians
    float myLatRad = PlanesObject::myLat * PI / 180;
    float myLonRad = PlanesObject::myLon * PI / 180;
    
    // First calculate the altitude angle to a temp array
    int tempArray[planeArraySize];
    // Serial.print("Distance array:");
    for (int i = 0; i < planeArraySize; i++) {
        // Calculate the altitude angle
        float planeLatRad = PlanesObject::planeLat[i] * PI / 180;
        float planeLonRad = PlanesObject::planeLon[i] * PI / 180;

        float a = sin((planeLatRad-myLatRad)/2)*sin((planeLatRad-myLatRad)/2) + cos(myLatRad)*cos(planeLatRad)*sin((planeLonRad-myLonRad)/2)*sin((planeLonRad-myLonRad)/2);
        float c = 2*atan2(sqrt(a), sqrt(1-a));
        float distance = 6371 * c;

        float altitudeAngle = atan2(PlanesObject::planeAltitude[i] - PlanesObject::myAlt, distance) * 180 / PI;

        tempArray[i] = altitudeAngle;
        // Serial.println(PlanesObject::planeName[i]);
        // Serial.println(String(PlanesObject::planeDistance[i]) + " ");
        // Serial.println(String(altitudeAngle) + " ");
    }
    Serial.println();

    // Create a pair array with the distance and the index of the plane
    std::pair<int, int> pairArray[planeArraySize];
    for (int i = 0; i < planeArraySize; i++) {
        pairArray[i] = std::make_pair(tempArray[i], i);
    }

    //Sort the pair array from largest to smallest
    std::sort(pairArray, pairArray + planeArraySize, std::greater<std::pair<int, int>>());

    // // Sort the pair array based on the distance
    // std::sort(pairArray, pairArray + planeArraySize);

    // Copy the sorted array to the sortedPlaneArray
    for (int i = 0; i < planeArraySize; i++) {
        PlanesObject::sortedPlaneArray[i] = pairArray[i].second;
    }

}

void PlanesObject::clearPlanes() {
    // Clear all plane information
    // Set all arrays to 0
    for (int i = 0; i < planeArraySize; i++) {
        PlanesObject::planeIdentifier[i] = "";
        PlanesObject::planeName[i] = "";
        PlanesObject::planeLat[i] = 0;
        PlanesObject::planeLon[i] = 0;
        PlanesObject::planeAltitude[i] = 0;
        PlanesObject::planeType[i] = "";
        PlanesObject::planeRegistration[i] = "";
        PlanesObject::planeHeading[i] = 0;
        PlanesObject::planeSpeed[i] = 0;
        PlanesObject::planeTimestamp[i] = 0;
        PlanesObject::planeDistance[i] = 99999;
        PlanesObject::sortedPlaneArray[i] = 0;
    }
    PlanesObject::planeCount = 0;
    PlanesObject::planeArrayFull = false;
}

void PlanesObject::printClosestPlane() {
    // Print the closest plane to the user
    // Print the plane information
    Serial.println("Closest plane:");
    Serial.println("Identifier: " + PlanesObject::planeIdentifier[PlanesObject::sortedPlaneArray[0]]);
    Serial.println("Name: " + PlanesObject::planeName[PlanesObject::sortedPlaneArray[0]]);
    Serial.println("Latitude: " + String(PlanesObject::planeLat[PlanesObject::sortedPlaneArray[0]]));
    Serial.println("Longitude: " + String(PlanesObject::planeLon[PlanesObject::sortedPlaneArray[0]]));
    Serial.println("Altitude: " + String(PlanesObject::planeAltitude[PlanesObject::sortedPlaneArray[0]]));
    Serial.println("Type: " + PlanesObject::planeType[PlanesObject::sortedPlaneArray[0]]);
    Serial.println("Registration: " + PlanesObject::planeRegistration[PlanesObject::sortedPlaneArray[0]]);
    Serial.println("Heading: " + String(PlanesObject::planeHeading[PlanesObject::sortedPlaneArray[0]]));
    Serial.println("Speed: " + String(PlanesObject::planeSpeed[PlanesObject::sortedPlaneArray[0]]));
    Serial.println("Timestamp: " + String(PlanesObject::planeTimestamp[PlanesObject::sortedPlaneArray[0]]));
    Serial.println("Distance: " + String(PlanesObject::planeDistance[PlanesObject::sortedPlaneArray[0]]));
}