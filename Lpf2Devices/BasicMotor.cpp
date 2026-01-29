#include "BasicMotor.h"

namespace
{
    BasicMotorFactory factory;
}

void BasicMotor::registerFactory(Lpf2DeviceRegistry& reg)
{
    reg.registerFactory(&factory);
}

void BasicMotor::setSpeed(int speed)
{
    assert(this);
    assert(((uintptr_t)this & 0x3) == 0);
    assert(port_.deviceConnected()); 
    bool forward = speed >= 0;
    speed = std::abs(speed);
    if (speed > 100)
        speed = 100;

    uint8_t pwr2 = speed * 0xFF / 100;
    uint8_t pwr1 = 0;
    if (!forward)
        std::swap(pwr1, pwr2);

    port_.setPower(pwr1, pwr2);
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