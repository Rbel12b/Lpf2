#include "Lpf2/Devices/BasicMotor.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        BasicMotorFactory factory;
    }

    void BasicMotor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    void BasicMotor::startPower(int speed)
    {
        m_port.startPower(speed);
    }

    bool BasicMotor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *BasicMotor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<BasicMotorControl *>(this);
        return nullptr;
    }

    bool BasicMotorFactory::matches(Port &port) const
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
}; // namespace Lpf2::Devices