#include "DistanceSensor.h"

const int TechnicDistanceSensor::LIGHT_MODE = 5;

const CapabilityId TechnicDistanceSensor::CAP =
    CapabilityRegistry::instance().registerCapability("technic_distance_sensor");

namespace
{
    TechnicDistanceSensorFactory factory;

    const bool registered = []
    {
        Lpf2DeviceRegistry::instance().registerFactory(&factory);
        return true;
    }();
}

void TechnicDistanceSensor::setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4)
{
    std::vector<uint8_t> data;
    if (l1 > 100)
        l1 = 100;
    if (l2 > 100)
        l2 = 100;
    if (l3 > 100)
        l3 = 100;
    if (l4 > 100)
        l4 = 100;

    data.push_back(l1);
    data.push_back(l2);
    data.push_back(l3);
    data.push_back(l4);

    port_.writeData(LIGHT_MODE, data);
}

float TechnicDistanceSensor::getDistance()
{
    return port_.getValue(0, 0);
}

bool TechnicDistanceSensor::hasCapability(CapabilityId id) const
{
    return id == CAP;
}

void *TechnicDistanceSensor::getCapability(CapabilityId id)
{
    if (id == CAP)
        return static_cast<TechnicDistanceSensorControl *>(this);
    return nullptr;
}

bool TechnicDistanceSensorFactory::matches(Lpf2Port &port) const
{
    switch (port.getDeviceType())
    {
    case Lpf2DeviceType::TECHNIC_DISTANCE_SENSOR:
        return true;
    default:
        break;
    }
    return false;
}
