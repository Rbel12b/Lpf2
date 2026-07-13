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
        return id == CAP;
    }

    void *ColorDistanceSensor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<ColorDistanceSensorControl *>(this);
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

    ColorIDX ColorDistanceSensor::getColorIdx()
    {
        return ColorIDX((int)m_port.getValue(0, 0));
    }

    float ColorDistanceSensor::getDistance()
    {
        return m_port.getValue(1, 0);
    }

    uint8_t ColorDistanceSensor::getReflectedLight()
    {
        return static_cast<uint8_t>(m_port.getValue(3, 0));
    }

    uint8_t ColorDistanceSensor::getAmbientLight()
    {
        return static_cast<uint8_t>(m_port.getValue(4, 0));
    }

    void ColorDistanceSensor::getRgb(uint16_t &red, uint16_t &green, uint16_t &blue)
    {
        red = static_cast<uint16_t>(m_port.getValue(6, 0));
        green = static_cast<uint16_t>(m_port.getValue(6, 1));
        blue = static_cast<uint16_t>(m_port.getValue(6, 2));
    }

    void ColorDistanceSensor::setIrTx(uint16_t value)
    {
        m_port.writeData(7, {static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF)});
    }

    void ColorDistanceSensor::setLedColor(ColorIDX color)
    {
        m_port.writeData(5, {(uint8_t)color});
    }
}; // namespace Lpf2::Devices
