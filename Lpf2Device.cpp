#include "Lpf2Device.h"
#include "Lpf2Devices/EncoderMotor.h"
#include "Lpf2Devices/BasicMotor.h"
#include "Lpf2Devices/ColorSensor.h"
#include "Lpf2Devices/DistanceSensor.h"

void Lpf2DeviceRegistry::registerDefaultFactories()
{
    // Order matters
    EncoderMotor::registerFactory(Lpf2DeviceRegistry::instance()); // EncoderMotor must be before BasicMotor, because we prefer EncoderMotors over BasicMotors
    BasicMotor::registerFactory(Lpf2DeviceRegistry::instance());
    // TODO: Add colordistanceSensor here! (above Color and Distance sensors)
    TechnicColorSensor::registerFactory(Lpf2DeviceRegistry::instance());
    TechnicDistanceSensor::registerFactory(Lpf2DeviceRegistry::instance());
}