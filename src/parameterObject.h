//Object to save, load and update parameters

#ifndef parameterObject_h
#define parameterObject_h

#include <Arduino.h>
#include "Preferences.h"

class ParameterObject {
public:
    // Functions for the parameter object
    void init();
    void saveParameters(bool defaultValues = false);
    void loadParameters();

    void putString(const char* key, const String value){
        preferences.putString(key, value);
    }
    void putFloat(const char* key, const float value){
        preferences.putFloat(key, value);
    }
    void putInt(const char* key, const int value){
        preferences.putInt(key, value);
    }

    String getString(const char* key, const String defaultValue){
        return preferences.getString(key, defaultValue);
    }
    float getFloat(const char* key, const float defaultValue){
        return preferences.getFloat(key, defaultValue);
    }
    int getInt(const char* key, const int defaultValue){
        return preferences.getInt(key, defaultValue);
    }

    void remove(const char* key){
        preferences.remove(key);
    }

    // Variables for the parameter object
    float areaMaxLat;
    float areaMinLat;
    float areaMaxLon;
    float areaMinLon;
    float myLat;
    float myLon;
    float myAlt;
    String ssid;
    String password;

    // Boolean to tell if the parameters have been updated
    bool parametersUpdated = false;

    // Variables for the preferences object
    Preferences preferences;
};

#endif // parameterObject_h