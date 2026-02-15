#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"

namespace Lpf2::Devices
{
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

    class TechnicDistanceSensor : public Device, public TechnicDistanceSensorControl
    {
    public:
        TechnicDistanceSensor(Port &port) : Device(port) {}

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

        inline static const int LIGHT_MODE = 5;

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("technic_distance_sensor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class TechnicDistanceSensorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new TechnicDistanceSensor(port);
        }

        const char *name() const
        {
            return "Technic Distance Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices