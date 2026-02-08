#pragma once
#ifndef _LPF2_COLOR_SENSOR_H_
#define _LPF2_COLOR_SENSOR_H_

#include "../config.hpp"
#include "../Lpf2Device.hpp"


class TechnicColorSensorControl
{
public:
    virtual ~TechnicColorSensorControl() = default;
    /**
     * @brief Get the color idx from the sensor.
     */
    virtual Lpf2Color getColorIdx() = 0;
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

    Lpf2Color getColorIdx() override;

    bool hasCapability(Lpf2DeviceCapabilityId id) const override;
    void *getCapability(Lpf2DeviceCapabilityId id) override;


    inline static const Lpf2DeviceCapabilityId CAP =
        Lpf2CapabilityRegistry::registerCapability("technic_color_sensor");

    static void registerFactory(Lpf2DeviceRegistry& reg);
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