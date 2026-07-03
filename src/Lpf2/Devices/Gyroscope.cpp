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

#include "Lpf2/Devices/Gyroscope.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        HubGyroscopeFactory factory;
    }

    void HubGyroscope::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    bool HubGyroscope::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *HubGyroscope::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<HubGyroscopeControl *>(this);
        return nullptr;
    }

    bool HubGyroscopeFactory::matches(const Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::TECHNIC_MEDIUM_HUB_GYRO_SENSOR:
            return true;
        default:
            break;
        }
        return false;
    }

    float HubGyroscope::getX()
    {
        return m_port.getValue(0, 0);
    }

    float HubGyroscope::getY()
    {
        return m_port.getValue(0, 1);
    }

    float HubGyroscope::getZ()
    {
        return m_port.getValue(0, 2);
    }
}; // namespace Lpf2::Devices
