#include "Lpf2/Devices/ColorSensor.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        TechnicColorSensorFactory factory;
    }

    void TechnicColorSensor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    ColorIDX TechnicColorSensor::getColorIdx()
    {
        return ColorIDX((int)m_port.getValue(0, 0));
    }

    bool TechnicColorSensor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *TechnicColorSensor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<TechnicColorSensorControl *>(this);
        return nullptr;
    }

    bool TechnicColorSensorFactory::matches(Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::TECHNIC_COLOR_SENSOR:
            return true;
        default:
            break;
        }
        return false;
    }
}; // namespace Lpf2::Devices