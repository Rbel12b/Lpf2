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
    class HubGyroscopeControl
    {
    public:
        virtual ~HubGyroscopeControl() = default;

        /**
         * @brief Get the X-axis angular velocity (mode 0 = ROT, unit: DPS).
         */
        virtual float getX() = 0;

        /**
         * @brief Get the Y-axis angular velocity (mode 0 = ROT, unit: DPS).
         */
        virtual float getY() = 0;

        /**
         * @brief Get the Z-axis angular velocity (mode 0 = ROT, unit: DPS).
         */
        virtual float getZ() = 0;
    };

    class HubGyroscope : public PortDevice, public HubGyroscopeControl
    {
    public:
        HubGyroscope(Port &port) : PortDevice(port) {}

        bool init() override
        {
            return true;
        }

        void update() override
        {
        }

        const char *name() const override
        {
            return "Hub Gyroscope";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("hub_gyroscope");

        static void registerFactory(DeviceRegistry &reg);

        float getX() override;
        float getY() override;
        float getZ() override;
    };

    class HubGyroscopeFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new HubGyroscope(port);
        }

        const char *name() const
        {
            return "Hub Gyroscope Factory";
        }
    };
}; // namespace Lpf2::Devices
