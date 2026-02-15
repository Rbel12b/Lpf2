#include "Lpf2/Devices/ColorSensor.hpp"

namespace
{
    TechnicColorSensorFactory factory;
}

void TechnicColorSensor::registerFactory(Lpf2DeviceRegistry& reg)
{
    reg.registerFactory(&factory);
}

Lpf2Color TechnicColorSensor::getColorIdx()
{
    return Lpf2Color((int)m_port.getValue(0, 0));
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
