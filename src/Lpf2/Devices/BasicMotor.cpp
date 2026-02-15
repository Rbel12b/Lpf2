#include "Lpf2/Devices/BasicMotor.hpp"

namespace
{
    BasicMotorFactory factory;
}

void BasicMotor::registerFactory(Lpf2DeviceRegistry& reg)
{
    reg.registerFactory(&factory);
}

void BasicMotor::startPower(int speed)
{
    m_port.startPower(speed);
}

bool BasicMotor::hasCapability(Lpf2DeviceCapabilityId id) const
{
    return id == CAP;
}

void *BasicMotor::getCapability(Lpf2DeviceCapabilityId id)
{
    if (id == CAP)
        return static_cast<BasicMotorControl *>(this);
    return nullptr;
}

bool BasicMotorFactory::matches(Lpf2Port &port) const
{
    switch (port.getDeviceType())
    {
    case Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
    case Lpf2DeviceType::TRAIN_MOTOR:
        return true;
    default:
        break;
    }
    return false;
}