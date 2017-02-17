// MayFlyOnboardSensors.h

#ifndef _MayFlyOnboardSensors_h
#define _MayFlyOnboardSensors_h
#include "Arduino.h"

#include "Sensor.h"

// The main class for the Mayfly
class MayFlyOnboardSensors : public virtual SensorBase
{
public:
    MayFlyOnboardSensors(void);
    virtual ~MayFlyOnboardSensors(void);

    bool update(void) override;
    String getSensorName(void) override;

    virtual String getVarName(void) = 0;
    virtual String getVarUnit(void) = 0;
    virtual float getValue(void) = 0;
protected:
    SENSOR_STATUS sensorStatus;
    String sensorName;
    String varName;
    String unit;
    int _batteryPin;
    float sensorValue_temp;
    float sensorValue_battery;
};


// Defines the "Temperature Sensor"
class MayFlyOnboardTemp : public virtual MayFlyOnboardSensors
{
public:
    MayFlyOnboardTemp(void);
    ~MayFlyOnboardTemp(void);

    String getVarName(void) override;
    String getVarUnit(void) override;
    float getValue(void) override;
};


// Defines the "Battery Sensor"
class MayFlyOnboardBatt : public virtual MayFlyOnboardSensors
{
public:
    MayFlyOnboardBatt(void);
    ~MayFlyOnboardBatt(void);

    String getVarName(void) override;
    String getVarUnit(void) override;
    float getValue(void) override;
};

#endif