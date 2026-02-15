#include "Lpf2/Device.hpp"
#include "Lpf2/Devices/EncoderMotor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/DistanceSensor.hpp"

void Lpf2DeviceRegistry::registerDefault()
{
    // Order matters
    EncoderMotor::registerFactory(Lpf2DeviceRegistry::instance()); // EncoderMotor must be before BasicMotor, because we prefer EncoderMotors over BasicMotors
    BasicMotor::registerFactory(Lpf2DeviceRegistry::instance());
    // TODO: Add colordistanceSensor here! (above Color and Distance sensors)
    TechnicColorSensor::registerFactory(Lpf2DeviceRegistry::instance());
    TechnicDistanceSensor::registerFactory(Lpf2DeviceRegistry::instance());
}