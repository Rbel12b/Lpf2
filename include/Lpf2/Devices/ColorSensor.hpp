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
#include "Lpf2/Device.hpp"

namespace Lpf2::Devices
{
    class TechnicColorSensorControl
    {
    public:
        virtual ~TechnicColorSensorControl() = default;
        /**
         * @brief Get the color idx from the sensor.
         */
        virtual ColorIDX getColorIdx() = 0;
    };

    class TechnicColorSensor : public Device, public TechnicColorSensorControl
    {
    public:
        TechnicColorSensor(Port &port) : Device(port) {}

        bool init() override
        {
            return true;
        }

        void poll() override
        {
        }

        const char *name() const override
        {
            return "Technic Color Sensor";
        }

        ColorIDX getColorIdx() override;

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("technic_color_sensor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class TechnicColorSensorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new TechnicColorSensor(port);
        }

        const char *name() const
        {
            return "Technic Color Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices