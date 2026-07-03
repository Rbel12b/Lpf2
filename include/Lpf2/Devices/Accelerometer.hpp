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
    class HubAccelerometerControl
    {
    public:
        virtual ~HubAccelerometerControl() = default;

        /**
         * @brief Get the X-axis acceleration (mode 0 = GRV, unit: mG).
         */
        virtual float getX() = 0;

        /**
         * @brief Get the Y-axis acceleration (mode 0 = GRV, unit: mG).
         */
        virtual float getY() = 0;

        /**
         * @brief Get the Z-axis acceleration (mode 0 = GRV, unit: mG).
         */
        virtual float getZ() = 0;
    };

    class HubAccelerometer : public PortDevice, public HubAccelerometerControl
    {
    public:
        HubAccelerometer(Port &port) : PortDevice(port) {}

        bool init() override
        {
            return true;
        }

        void update() override
        {
        }

        const char *name() const override
        {
            return "Hub Accelerometer";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("hub_accelerometer");

        static void registerFactory(DeviceRegistry &reg);

        float getX() override;
        float getY() override;
        float getZ() override;
    };

    class HubAccelerometerFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new HubAccelerometer(port);
        }

        const char *name() const
        {
            return "Hub Accelerometer Factory";
        }
    };
}; // namespace Lpf2::Devices
