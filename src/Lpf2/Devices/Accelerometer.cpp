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

#include "Lpf2/Devices/Accelerometer.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        HubAccelerometerFactory factory;
    }

    void HubAccelerometer::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    bool HubAccelerometer::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *HubAccelerometer::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<HubAccelerometerControl *>(this);
        return nullptr;
    }

    bool HubAccelerometerFactory::matches(const Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::TECHNIC_MEDIUM_HUB_ACCELEROMETER:
            return true;
        default:
            break;
        }
        return false;
    }

    float HubAccelerometer::getX()
    {
        return m_port.getValue(0, 0);
    }

    float HubAccelerometer::getY()
    {
        return m_port.getValue(0, 1);
    }

    float HubAccelerometer::getZ()
    {
        return m_port.getValue(0, 2);
    }
}; // namespace Lpf2::Devices
