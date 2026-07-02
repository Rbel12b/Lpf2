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

#include "Lpf2/Devices/HubLED.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        HubLEDFactory factory;
    }

    void HubLED::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    void HubLED::setColorIdx(ColorIDX color)
    {
        m_port.writeData(0, {static_cast<uint8_t>(color)});
    }

    void HubLED::setColor(uint8_t r, uint8_t g, uint8_t b)
    {
        m_port.writeData(1, {r, g, b});
    }

    bool HubLED::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *HubLED::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<HubLEDControl *>(this);
        return nullptr;
    }

    bool HubLEDFactory::matches(const Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::HUB_LED:
            return true;
        default:
            break;
        }
        return false;
    }
}; // namespace Lpf2::Devices