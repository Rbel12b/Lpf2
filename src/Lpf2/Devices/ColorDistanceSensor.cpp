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

#include "Lpf2/Devices/ColorDistanceSensor.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        ColorDistanceSensorFactory factory;
    }

    void ColorDistanceSensor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    bool ColorDistanceSensor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP
            || id == TechnicColorSensor::CAP
            || id == TechnicDistanceSensor::CAP;
    }

    void *ColorDistanceSensor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<ColorDistanceSensorControl *>(this);
        else if (id == TechnicColorSensor::CAP)
            return static_cast<TechnicColorSensorControl *>(this);
        else if (id == TechnicDistanceSensor::CAP)
            return static_cast<TechnicDistanceSensorControl *>(this);
        return nullptr;
    }

    bool ColorDistanceSensorFactory::matches(const Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::COLOR_DISTANCE_SENSOR:
            return true;
        default:
            break;
        }
        return false;
    }

    bool ColorDistanceSensor::init()
    {
        return m_port.setModeCombo(0) == 0; // 0x004F
    }

    void ColorDistanceSensor::update()
    {
        return;
    }

    ColorIDX ColorDistanceSensor::getColorIdx()
    {
        if (!m_comboActive || m_modeActive != MODE_COLOR)
        {
            setMode(MODE_COLOR);
        }

        return ColorIDX((int)m_port.getValue(MODE_COLOR, 0));
    }

    float ColorDistanceSensor::getDistance()
    {
        if (!m_comboActive || m_modeActive != MODE_DIST)
        {
            setMode(MODE_DIST);
        }

        return m_port.getValue(MODE_DIST, 0);
    }

    float ColorDistanceSensor::getReflectivity()
    {
        if (!m_comboActive || m_modeActive != MODE_REFLT)
        {
            setMode(MODE_REFLT);
        }

        return m_port.getValue(MODE_REFLT, 0);
    }

    float ColorDistanceSensor::getAmbientLight()
    {
        if (!m_comboActive || m_modeActive != MODE_AMBI)
        {
            setMode(MODE_AMBI);
        }

        return m_port.getValue(MODE_AMBI, 0);
    }

    void ColorDistanceSensor::getRGB(uint16_t &r, uint16_t &g, uint16_t &b)
    {
        if (!m_comboActive || m_modeActive != MODE_RGB)
        {
            setMode(MODE_RGB);
        }

        r = static_cast<uint16_t>(m_port.getValue(MODE_RGB, 0));
        g = static_cast<uint16_t>(m_port.getValue(MODE_RGB, 1));
        b = static_cast<uint16_t>(m_port.getValue(MODE_RGB, 2));
    }

    void ColorDistanceSensor::setIrTx(uint16_t value)
    {
        m_port.writeData(MODE_IR, {static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF)});
    }

    void ColorDistanceSensor::setLedColor(ColorIDX color)
    {
        m_port.writeData(MODE_LED, {(uint8_t)color});
    }

    int ColorDistanceSensor::setMode(uint8_t modeNum, float delta)
    {
        if (modeNum == 0 || modeNum == 1 || modeNum == 2 || modeNum == 3 || modeNum == 6)
        {
            m_comboActive = true;
            m_port.setModeCombo(0, {delta, delta, delta, delta, delta});
        }
        else
        {
            m_modeActive = modeNum;
            m_comboActive = false;
            m_port.setMode(modeNum, delta);
        }
    }
}; // namespace Lpf2::Devices
