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

    bool TechnicColorSensorFactory::matches(const Port &port) const
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

    ColorIDX TechnicColorSensor::getColorIdx()
    {
        return ColorIDX((int)m_port.getValue(0, 0));
    }

    float TechnicColorSensor::getReflectivity()
    {
        return m_port.getValue(1, 0);
    }

    void TechnicColorSensor::getRGB(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &i)
    {
        r = (uint16_t)m_port.getValue(5, 0);
        g = (uint16_t)m_port.getValue(5, 1);
        b = (uint16_t)m_port.getValue(5, 2);
        i = (uint16_t)m_port.getValue(5, 3);
    }

    void TechnicColorSensor::getHSV(uint16_t &h, uint16_t &s, uint16_t &v)
    {
        h = (uint16_t)m_port.getValue(6, 0);
        s = (uint16_t)m_port.getValue(6, 1);
        v = (uint16_t)m_port.getValue(6, 2);
    }

    void TechnicColorSensor::setLight(uint8_t l1, uint8_t l2, uint8_t l3)
    {
        if (l1 > 100)
            l1 = 100;
        if (l2 > 100)
            l2 = 100;
        if (l3 > 100)
            l3 = 100;

        std::vector<uint8_t> data;
        data.push_back(l1);
        data.push_back(l2);
        data.push_back(l3);

        m_port.writeData(LIGHT_MODE, data);
    }
}; // namespace Lpf2::Devices