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
    class ColorDistanceSensorControl
    {
    public:
        virtual ~ColorDistanceSensorControl() = default;

        virtual ColorIDX getColorIdx() = 0;
        virtual float getDistance() = 0;
        virtual uint8_t getReflectedLight() = 0;
        virtual uint8_t getAmbientLight() = 0;
        virtual void getRgb(uint16_t &red, uint16_t &green, uint16_t &blue) = 0;
        virtual void setIrTx(uint16_t value) = 0;
        virtual void setLedColor(ColorIDX color) = 0;
    };

    class ColorDistanceSensor : public PortDevice, public ColorDistanceSensorControl
    {
    public:
        ColorDistanceSensor(Port &port) : PortDevice(port) {}

        bool init() override
        {
            return true;
        }

        void update() override
        {
        }

        const char *name() const override
        {
            return "Color Distance Sensor";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("color_distance_sensor");

        static void registerFactory(DeviceRegistry &reg);

        ColorIDX getColorIdx() override;
        float getDistance() override;
        uint8_t getReflectedLight() override;
        uint8_t getAmbientLight() override;
        void getRgb(uint16_t &red, uint16_t &green, uint16_t &blue) override;
        void setIrTx(uint16_t value) override;
        void setLedColor(ColorIDX color) override;
    };

    class ColorDistanceSensorFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new ColorDistanceSensor(port);
        }

        const char *name() const override
        {
            return "Color Distance Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices
