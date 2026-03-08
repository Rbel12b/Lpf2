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
    class BasicMotorControl
    {
    public:
        virtual ~BasicMotorControl() = default;
        virtual void startPower(int speed) = 0;
    };

    class BasicMotor : public Device, public BasicMotorControl
    {
    public:
        BasicMotor(Port &port) : Device(port) {}

        bool init() override
        {
            startPower(0);
            return true;
        }

        void poll() override
        {
        }

        const char *name() const override
        {
            return "DC Motor (dumb)";
        }

        void startPower(int speed) override;

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("basic_motor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class BasicMotorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new BasicMotor(port);
        }

        const char *name() const
        {
            return "Basic Motor Factory";
        }
    };
}; // namespace Lpf2::Devices