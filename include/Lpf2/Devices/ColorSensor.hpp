#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"

namespace Lpf2::Devices
{
    class TechnicColorSensorControl
    {
    public:
        virtual ~TechnicColorSensorControl() = default;
        /**
         * @brief Get the color idx from the sensor.
         */
        virtual ColorIDX getColorIdx() = 0;
    };

    class TechnicColorSensor : public Device, public TechnicColorSensorControl
    {
    public:
        TechnicColorSensor(Port &port) : Device(port) {}

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

        ColorIDX getColorIdx() override;

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("technic_color_sensor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class TechnicColorSensorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new TechnicColorSensor(port);
        }

        const char *name() const
        {
            return "Technic Color Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices