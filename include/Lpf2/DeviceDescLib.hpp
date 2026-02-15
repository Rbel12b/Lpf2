#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"

namespace Lpf2DeviceDescriptors
{
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GEST_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TILT_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GYRO_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_ACCELEROMETER;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_HUB_LED;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TRAIN_MOTOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_CURRENT_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_VOLTAGE_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_ANGULAR_MOTOR_GREY;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_DISTANCE_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_COLOR_SENSOR;
    extern const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_LINEAR_MOTOR;
};

class Lpf2DeviceDescRegistry
{
public:
    static Lpf2DeviceDescRegistry &instance()
    {
        static Lpf2DeviceDescRegistry inst;
        return inst;
    }

    static void registerDefault();

    void registerDesc(Lpf2DeviceType type, const Lpf2DeviceDescriptor *desc)
    {
        if (count_ >= MAX_DESCRIPTORS)
        {
            assert(false && "Exceeded maximum number of Lpf2 device factories");
            return;
        }

        _descriptors[count_++] = Lpf2DeviceDescRegistryEntry{desc, type};
    }

    const Lpf2DeviceDescriptor *getDescriptor(Lpf2DeviceType type)
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
        const Lpf2DeviceDescriptor* _desc;
        Lpf2DeviceType _type;
    }_descriptors[MAX_DESCRIPTORS];
    size_t count_ = 0;
};