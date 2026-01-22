#include "ColorSensor.h"

const CapabilityId TechnicColorSensor::CAP =
    CapabilityRegistry::instance().registerCapability("technic_color_sensor");

namespace
{
    TechnicColorSensorFactory factory;

    const bool registered = []
    {
        Lpf2DeviceRegistry::instance().registerFactory(&factory);
        return true;
    }();
}

Lpf2ColorIDX TechnicColorSensor::getColorIdx()
{
    return Lpf2ColorIDX((int)port_.getValue(0, 0));
}

bool TechnicColorSensor::hasCapability(CapabilityId id) const
{
    return id == CAP;
}

void *TechnicColorSensor::getCapability(CapabilityId id)
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
