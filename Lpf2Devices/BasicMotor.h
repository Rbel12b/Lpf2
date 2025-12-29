#pragma once
#ifndef _LPF2_BASIC_MOTOR_H_
#define _LPF2_BASIC_MOTOR_H_

#include "../config.h"
#include "../Lpf2Device.h"

class BasicMotorControl
{
public:
    virtual ~BasicMotorControl() = default;
    virtual void setSpeed(int speed) = 0;
};

class BasicMotor : public Lpf2Device, public BasicMotorControl
{
public:
    BasicMotor(Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override
    {
        setSpeed(0);
        return true;
    }

    void poll() override
    {
    }

    const char *name() const override
    {
        return "DC Motor (dumb)";
    }

    void setSpeed(int speed) override;

    bool hasCapability(CapabilityId id) const override;
    void *getCapability(CapabilityId id) override;

    const static CapabilityId CAP;
};

class BasicMotorFactory : public Lpf2DeviceFactory
{
public:
    bool matches(Lpf2Port &port) const override;

    Lpf2Device *create(Lpf2Port &port) const override
    {
        return new BasicMotor(port);
    }

    const char *name() const
    {
        return "Basic Motor Factory";
    }
};

#endif