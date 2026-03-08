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

#include "Lpf2/Devices/DistanceSensor.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        TechnicDistanceSensorFactory factory;
    }

    void TechnicDistanceSensor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
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

        m_port.writeData(LIGHT_MODE, data);
    }

    float TechnicDistanceSensor::getDistance()
    {
        return m_port.getValue(0, 0);
    }

    bool TechnicDistanceSensor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *TechnicDistanceSensor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<TechnicDistanceSensorControl *>(this);
        return nullptr;
    }

    bool TechnicDistanceSensorFactory::matches(Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::TECHNIC_DISTANCE_SENSOR:
            return true;
        default:
            break;
        }
        return false;
    }
}; // namespace Lpf2::Devices