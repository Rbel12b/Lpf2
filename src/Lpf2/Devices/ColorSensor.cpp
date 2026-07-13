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

    bool TechnicColorSensor::init()
    {
        m_port.setModeCombo(0); //0x0063
        m_comboActive = true;
        return true;
    }

    void TechnicColorSensor::update()
    {
    }

    ColorIDX TechnicColorSensor::getColorIdx()
    {
        if (!m_comboActive || m_modeActive != MODE_COLOR)
        {
            setMode(MODE_COLOR);
        }

        return ColorIDX((int)m_port.getValue(MODE_COLOR, 0));
    }

    float TechnicColorSensor::getReflectivity()
    {
        if (!m_comboActive || m_modeActive != MODE_REFLT)
        {
            setMode(MODE_REFLT);
        }

        return m_port.getValue(MODE_REFLT, 0);
    }

    float TechnicColorSensor::getAmbientLight()
    {
        if (m_modeActive != MODE_AMBI)
        {
            setMode(MODE_AMBI);
        }

        return m_port.getValue(MODE_AMBI, 0);
    }

    void TechnicColorSensor::getRGB(uint16_t &r, uint16_t &g, uint16_t &b)
    {
        if (!m_comboActive || m_modeActive != MODE_RGB)
        {
            setMode(MODE_RGB);
        }

        r = (uint16_t)m_port.getValue(MODE_RGB, 0);
        g = (uint16_t)m_port.getValue(MODE_RGB, 1);
        b = (uint16_t)m_port.getValue(MODE_RGB, 2);
    }

    void TechnicColorSensor::getHSV(uint16_t &h, uint16_t &s, uint16_t &v)
    {
        if (!m_comboActive || m_modeActive != MODE_HSV)
        {
            setMode(MODE_HSV);
        }

        h = (uint16_t)m_port.getValue(MODE_HSV, 0);
        s = (uint16_t)m_port.getValue(MODE_HSV, 1);
        v = (uint16_t)m_port.getValue(MODE_HSV, 2);
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

        m_port.writeData(MODE_LIGHT, data);
    }

    int TechnicColorSensor::setMode(uint8_t modeNum, float delta)
    {
        if (modeNum == 0 || modeNum == 1 || modeNum == 5 || modeNum == 6)
        {
            m_comboActive = true;
            m_port.setModeCombo(0, {delta, delta, delta, delta});
        }
        else
        {
            m_modeActive = modeNum;
            m_comboActive = false;
            m_port.setMode(modeNum, delta);
        }
    }
}; // namespace Lpf2::Devices