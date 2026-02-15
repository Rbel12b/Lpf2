#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/DeviceDesc.hpp"

namespace Lpf2
{
    namespace DeviceDescriptors
    {
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GEST_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TILT_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GYRO_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_ACCELEROMETER;
        extern const DeviceDescriptor LPF2_DEVICE_HUB_LED;
        extern const DeviceDescriptor LPF2_DEVICE_TRAIN_MOTOR;
        extern const DeviceDescriptor LPF2_DEVICE_CURRENT_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_VOLTAGE_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_ANGULAR_MOTOR_GREY;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_DISTANCE_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_COLOR_SENSOR;
        extern const DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_LINEAR_MOTOR;
    };

    class DeviceDescRegistry
    {
    public:
        static DeviceDescRegistry &instance()
        {
            static DeviceDescRegistry inst;
            return inst;
        }

        static void registerDefault();

        void registerDesc(DeviceType type, const DeviceDescriptor *desc)
        {
            if (count_ >= MAX_DESCRIPTORS)
            {
                assert(false && "Exceeded maximum number of Lpf2 device factories");
                return;
            }

            _descriptors[count_++] = Lpf2DeviceDescRegistryEntry{desc, type};
        }

        const DeviceDescriptor *getDescriptor(DeviceType type)
        {
            for (size_t i = 0; i < count_; i++)
            {
                if (_descriptors[i]._type == type)
                {
                    return _descriptors[i]._desc;
                }
            }
            return nullptr;
        }

        size_t count() const
        {
            return count_;
        }

    private:
        static constexpr size_t MAX_DESCRIPTORS = 64;

        struct Lpf2DeviceDescRegistryEntry
        {
            const DeviceDescriptor *_desc;
            DeviceType _type;
        } _descriptors[MAX_DESCRIPTORS];
        size_t count_ = 0;
    };
}; // namespace Lpf2