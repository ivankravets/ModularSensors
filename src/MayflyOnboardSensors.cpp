/*
 *MayflyOnboardSensors.cpp
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This file is for the onboard "sensors" on the EnviroDIY Mayfly
 *It is dependent on the EnviroDIY DS3231 library.
*/

#include <Arduino.h>
#include "MayflyOnboardSensors.h"
#include <Sodaq_DS3231.h>

// No power pin to switch, only returns true
bool MayflyOnboardSensors::sleep(void)
{
    return true;
}

// No power pin to switch, only returns true
bool MayflyOnboardSensors::wake(void)
{
    return true;
}


// Resolution is 0.25°C
// Accuracy is ±3°C
MayflyOnboardTemp::MayflyOnboardTemp(char const *version)
  : SensorBase(-1, -1, F("EnviroDIYMayflyRTC"), F("temperatureDatalogger"), F("degreeCelsius"), 2, F("BoardTemp"))
{ _version = version; }
// The location of the sensor on the Mayfly
String MayflyOnboardTemp::getSensorLocation(void)
{
    sensorLocation = F("DS3231");
    return sensorLocation;
}

// How to update the onboard sensors
bool MayflyOnboardTemp::update(void)
{
    // Get the temperature from the Mayfly's real time clock
    rtc.convertTemperature();  //convert current temperature into registers
    float tempVal = rtc.getTemperature();
    sensorValue_temp = tempVal;
    sensorLastUpdated = millis();

    // Return true when finished
    return true;
}

float MayflyOnboardTemp::getValue(void)
{
    checkForUpdate(sensorLastUpdated);
    return sensorValue_temp;
}


// The constructor - needs to reference the super-class constructor//
// Range of 0-5V with 10bit ADC - resolution of 0.005
MayflyOnboardBatt::MayflyOnboardBatt(char const *version)
  : SensorBase(-1, -1, F("EnviroDIYMayflyBatt"), F("batteryVoltage"), F("Volt"), 3, F("Battery"))
{
    _version = version;

    if (strcmp(_version, "v0.3") == 0 or strcmp(_version, "v0.4") == 0)
    {
        // Set the pin to read the battery voltage
        _batteryPin = A6;
    }
    if (strcmp(_version, "v0.5") == 0)
    {
        // Set the pin to read the battery voltage
        _batteryPin = A6;
    }
}

// The location of the sensor on the Mayfly
String MayflyOnboardBatt::getSensorLocation(void)
{
    sensorLocation = String(_batteryPin);
    return sensorLocation;
}

// How to update the onboard sensors
bool MayflyOnboardBatt::update(void)
{
    if (strcmp(_version, "v0.3") == 0 or strcmp(_version, "v0.4") == 0)
    {
        // Get the battery voltage
        float rawBattery = analogRead(_batteryPin);
        sensorValue_battery = (3.3 / 1023.) * 1.47 * rawBattery;
        sensorLastUpdated = millis();
    }
    if (strcmp(_version, "v0.5") == 0)
    {
        // Get the battery voltage
        float rawBattery = analogRead(_batteryPin);
        sensorValue_battery = (3.3 / 1023.) * 4.7 * rawBattery;
        sensorLastUpdated = millis();
    }

    // Return true when finished
    return true;
}

float MayflyOnboardBatt::getValue(void)
{
    checkForUpdate(sensorLastUpdated);
    return sensorValue_battery;
}




// The constructor - needs to reference the super-class constructor
// Interger value
MayflyFreeRam::MayflyFreeRam(void)
  : SensorBase(-1, -1, F("EnviroDIYMayflyHeap"), F("Free SRAM"), F("Bit"), 0, F("FreeRam"))
{}

// The location of the sensor on the Mayfly
String MayflyFreeRam::getSensorLocation(void)
{
    sensorLocation = "AtMega1284P";
    return sensorLocation;
}

// How to update the onboard sensors
bool MayflyFreeRam::update(void)
{
    // Used only for debugging - can be removed
      extern int __heap_start, *__brkval;
      int v;
      sensorValue_freeRam = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
      sensorLastUpdated = millis();

    // Return true when finished
    return true;
}

float MayflyFreeRam::getValue(void)
{
    checkForUpdate(sensorLastUpdated);
    return sensorValue_freeRam;
}
