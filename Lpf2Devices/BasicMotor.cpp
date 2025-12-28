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
}