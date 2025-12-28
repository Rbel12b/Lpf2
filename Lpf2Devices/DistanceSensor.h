#pragma once
#ifndef _LPF2_DISTANCE_SENSOR_H_
#define _LPF2_DISTANCE_SENSOR_H_

#include "../config.h"
#include "../Lpf2Device.h"

class DistanceSensorControl {
public:
    virtual ~DistanceSensorControl() = default;
    /**
     * @brief Set the light on the sensor, all values should be in the range 0-100.
     */
    virtual void setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4) = 0;
    /**
     * @brief Get the distance measured by the sensor in centimeters.
     */
    virtual float getDistance() = 0;
};

class DistanceSensor : public Lpf2Device, public DistanceSensorControl {
public:
    DistanceSensor (Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override {
        setLight(0, 0, 0, 0);
        return true;
    }

    void poll() override {
    }

    const char* name() const override {
        return "Technic Distance Sensor";
    }

    void setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4);
    float getDistance();

    const int LIGHT_MODE = 5;
};

class DistanceSensorFactory : public Lpf2DeviceFactory {
public:
    bool matches(Lpf2Port& port) const override;

    Lpf2Device* create(Lpf2Port& port) const override {
        return new DistanceSensor(port);
    }
};

#endif