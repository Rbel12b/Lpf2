#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"

namespace Lpf2::Devices
{
    class BasicMotorControl
    {
    public:
        virtual ~BasicMotorControl() = default;
        virtual void startPower(int speed) = 0;
    };

    class BasicMotor : public Device, public BasicMotorControl
    {
    public:
        BasicMotor(Port &port) : Device(port) {}

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

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("basic_motor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class BasicMotorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new BasicMotor(port);
        }

        const char *name() const
        {
            return "Basic Motor Factory";
        }
    };
}; // namespace Lpf2::Devices