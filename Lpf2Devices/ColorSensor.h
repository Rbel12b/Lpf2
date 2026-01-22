#pragma once
#ifndef _LPF2_COLOR_SENSOR_H_
#define _LPF2_COLOR_SENSOR_H_

#include "../config.h"
#include "../Lpf2Device.h"

enum class Lpf2ColorIDX
{
    BLACK = 0,
    PINK = 1,
    PURPLE = 2,
    BLUE = 3,
    LIGHTBLUE = 4,
    CYAN = 5,
    GREEN = 6,
    YELLOW = 7,
    ORANGE = 8,
    RED = 9,
    WHITE = 10,
    NUM_COLORS,
    NONE = 255
};

class TechnicColorSensorControl
{
public:
    virtual ~TechnicColorSensorControl() = default;
    /**
     * @brief Get the color idx from the sensor.
     */
    virtual Lpf2ColorIDX getColorIdx() = 0;
};

class TechnicColorSensor : public Lpf2Device, public TechnicColorSensorControl
{
public:
    TechnicColorSensor(Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override
    {
        return true;
    }

    void poll() override
    {
    }

    const char *name() const override
    {
        return "Technic Color Sensor";
    }

    Lpf2ColorIDX getColorIdx() override;

    bool hasCapability(CapabilityId id) const override;
    void *getCapability(CapabilityId id) override;

    const static CapabilityId CAP;
};

class TechnicColorSensorFactory : public Lpf2DeviceFactory
{
public:
    bool matches(Lpf2Port &port) const override;

    Lpf2Device *create(Lpf2Port &port) const override
    {
        return new TechnicColorSensor(port);
    }

    const char *name() const
    {
        return "Technic Color Sensor Factory";
    }
};

#endif