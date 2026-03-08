/**
 *  Copyright (C) 2026 - Rbel12b
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  */

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