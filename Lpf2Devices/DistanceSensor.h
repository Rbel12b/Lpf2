#pragma once
#ifndef _LPF2_DISTANCE_SENSOR_H_
#define _LPF2_DISTANCE_SENSOR_H_

#include "../config.h"
#include "../Lpf2Device.h"

class TechnicDistanceSensorControl
{
public:
    virtual ~TechnicDistanceSensorControl() = default;
    /**
     * @brief Set the light on the sensor, all values should be in the range 0-100.
     */
    virtual void setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4) = 0;
    /**
     * @brief Get the distance measured by the sensor in centimeters.
     */
    virtual float getDistance() = 0;
};

class TechnicDistanceSensor : public Lpf2Device, public TechnicDistanceSensorControl
{
public:
    TechnicDistanceSensor(Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override
    {
        setLight(0, 0, 0, 0);
        return true;
    }

    void poll() override
    {
    }

    const char *name() const override
    {
        return "Technic Distance Sensor";
    }

    void setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4);
    float getDistance();

    const static int LIGHT_MODE;

    bool hasCapability(CapabilityId id) const override;
    void *getCapability(CapabilityId id) override;

    const static CapabilityId CAP;
};

class TechnicDistanceSensorFactory : public Lpf2DeviceFactory
{
public:
    bool matches(Lpf2Port &port) const override;

    Lpf2Device *create(Lpf2Port &port) const override
    {
        return new TechnicDistanceSensor(port);
    }

    const char *name() const
    {
        return "Technic Distance Sensor Factory";
    }
};

#endif