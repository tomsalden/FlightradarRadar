//Functions for parameterObject.cpp:

#include "parameterObject.h"
#include "variables.h"

void ParameterObject::init() {
    // Initialize the parameter object
    // Open the preferences object
    preferences.begin("flightradar", false);
    if (preferences.getBool("Saved", false) == false){ //If no preferences are saved, save the default preferences
        saveParameters(true);
    } else { //Preferences have been saved, load them
        loadParameters();
    }
}

void ParameterObject::saveParameters(bool defaultValues) {
    // Save the parameters to the preferences object
    if (defaultValues == true){
        // Save the default values
        preferences.putFloat("areaMaxLat", default_areaMaxLat);
        preferences.putFloat("areaMinLat", default_areaMinLat);
        preferences.putFloat("areaMaxLon", default_areaMaxLon);
        preferences.putFloat("areaMinLon", default_areaMinLon);
        preferences.putFloat("myLat", default_myLat);
        preferences.putFloat("myLon", default_myLon);
        preferences.putFloat("myAlt", default_myAlt);
        preferences.putString("ssid", default_ssid);
        preferences.putString("password", default_password);
        preferences.putBool("Saved", true);
    } else {
        // Save the current values
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
    }
}

void ParameterObject::loadParameters() {
    // Load the parameters from the preferences object
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

