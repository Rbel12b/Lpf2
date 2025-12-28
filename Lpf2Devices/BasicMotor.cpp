#include "BasicMotor.h"

namespace {
    BasicMotorFactory basicMotorFactory;
}

static bool dcMotorRegistered = [] {
    Lpf2DeviceRegistry::instance().registerFactory(&basicMotorFactory);
    return true;
}();

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

bool BasicMotorFactory::matches(Lpf2Port &port) const
{
    switch (port.getDeviceType())
    {
    case DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
    case DeviceType::TRAIN_MOTOR:
        return true;
    default:
        break;
    }
    return false;
}