#pragma once
#ifndef _LPF2_BASIC_MOTOR_H_
#define _LPF2_BASIC_MOTOR_H_

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"

class BasicMotorControl
{
public:
    virtual ~BasicMotorControl() = default;
    virtual void startPower(int speed) = 0;
};

class BasicMotor : public Lpf2Device, public BasicMotorControl
{
public:
    BasicMotor(Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override
    {
        startPower(0);
        return true;
    }

    void poll() override
    {
    }

    const char *name() const override
    {
        return "DC Motor (dumb)";
    }

    void startPower(int speed) override;

    bool hasCapability(Lpf2DeviceCapabilityId id) const override;
    void *getCapability(Lpf2DeviceCapabilityId id) override;

    inline static const Lpf2DeviceCapabilityId CAP =
        Lpf2CapabilityRegistry::registerCapability("basic_motor");

    static void registerFactory(Lpf2DeviceRegistry& reg);
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