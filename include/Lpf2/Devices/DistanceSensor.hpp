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
    class TechnicDistanceSensorControl
    {
    public:
        virtual ~TechnicDistanceSensorControl() = default;
        /**
         * @brief Set the light on the sensor, all values should be in the range 0-100.
         */
        virtual void setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4) = 0;
        /**
         * @brief Get the distance measured by the sensor in centimeters.
         */
        virtual float getDistance() = 0;
    };

    class TechnicDistanceSensor : public Device, public TechnicDistanceSensorControl
    {
    public:
        TechnicDistanceSensor(Port &port) : Device(port) {}

        bool init() override
        {
            setLight(0, 0, 0, 0);
            return true;
        }

        void poll() override
        {
        }

        const char *name() const override
        {
            return "Technic Distance Sensor";
        }

        void setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4);
        float getDistance();

        inline static const int LIGHT_MODE = 5;

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("technic_distance_sensor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class TechnicDistanceSensorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new TechnicDistanceSensor(port);
        }

        const char *name() const
        {
            return "Technic Distance Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices