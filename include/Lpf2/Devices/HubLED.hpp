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

#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/DeviceFactory.hpp"

namespace Lpf2::Devices
{
    class HubLEDControl
    {
    public:
        virtual ~HubLEDControl() = default;
        virtual void setColorIdx(ColorIDX color) = 0;
        virtual void setColor(uint8_t r, uint8_t g, uint8_t b) = 0;
    };

    class HubLED : public PortDevice, public HubLEDControl
    {
    public:
        HubLED(Port &port) : PortDevice(port) {}

        bool init() override
        {
            return true;
        }

        void update() override
        {
        }

        const char *name() const override
        {
            return "Hub LED";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("hub_led");

        static void registerFactory(DeviceRegistry &reg);

        void setColorIdx(ColorIDX color) override;
        void setColor(uint8_t r, uint8_t g, uint8_t b) override;
    };

    class HubLEDFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new HubLED(port);
        }

        const char *name() const
        {
            return "Hub LED Factory";
        }
    };
}; // namespace Lpf2::Devices