#include "Lpf2/Device.hpp"
#include "Lpf2/Devices/EncoderMotor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/DistanceSensor.hpp"

namespace Lpf2
{
    using namespace Lpf2::Devices;
    void DeviceRegistry::registerDefault()
    {
        // Order matters
        EncoderMotor::registerFactory(DeviceRegistry::instance()); // EncoderMotor must be before BasicMotor, because we prefer EncoderMotors over BasicMotors
        BasicMotor::registerFactory(DeviceRegistry::instance());
        // TODO: Add colordistanceSensor here! (above Color and Distance sensors)
        TechnicColorSensor::registerFactory(DeviceRegistry::instance());
        TechnicDistanceSensor::registerFactory(DeviceRegistry::instance());
    }
};