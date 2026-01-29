#include "ColorSensor.h"

namespace
{
    TechnicColorSensorFactory factory;
}

void TechnicColorSensor::registerFactory(Lpf2DeviceRegistry& reg)
{
    reg.registerFactory(&factory);
}

Lpf2ColorIDX TechnicColorSensor::getColorIdx()
{
    return Lpf2ColorIDX((int)port_.getValue(0, 0));
}

bool TechnicColorSensor::hasCapability(Lpf2DeviceCapabilityId id) const
{
    return id == CAP;
}

void *TechnicColorSensor::getCapability(Lpf2DeviceCapabilityId id)
{
    if (id == CAP)
        return static_cast<TechnicColorSensorControl *>(this);
    return nullptr;
}

bool TechnicColorSensorFactory::matches(Lpf2Port &port) const
{
    switch (port.getDeviceType())
    {
    case Lpf2DeviceType::TECHNIC_COLOR_SENSOR:
        return true;
    default:
        break;
    }
    return false;
}
