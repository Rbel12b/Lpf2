#include "BasicMotor.h"

const CapabilityId BasicMotor::CAP =
    CapabilityRegistry::instance().registerCapability("basic_motor");

namespace
{
    BasicMotorFactory factory;

    static bool registered = []
    {
        Lpf2DeviceRegistry::instance().registerFactory(&factory);
        return true;
    }();
}

void BasicMotor::setSpeed(int speed)
{
    bool forward = speed >= 0;
    speed = std::abs(speed);
    if (speed > 100)
        speed = 100;

    uint8_t pwr1 = speed * 0xFF / 100;
    uint8_t pwr2 = 0;
    if (!forward)
        std::swap(pwr1, pwr2);

    port_.setPower(pwr1, pwr2);
}

bool BasicMotor::hasCapability(CapabilityId id) const
{
    return id == CAP;
}

void *BasicMotor::getCapability(CapabilityId id)
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