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

#include "Lpf2/Devices/ColorSensor.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        TechnicColorSensorFactory factory;
    }

    void TechnicColorSensor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    ColorIDX TechnicColorSensor::getColorIdx()
    {
        return ColorIDX((int)m_port.getValue(0, 0));
    }

    bool TechnicColorSensor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *TechnicColorSensor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<TechnicColorSensorControl *>(this);
        return nullptr;
    }

    bool TechnicColorSensorFactory::matches(Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::TECHNIC_COLOR_SENSOR:
            return true;
        default:
            break;
        }
        return false;
    }
}; // namespace Lpf2::Devices