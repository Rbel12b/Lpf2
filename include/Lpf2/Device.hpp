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
#include "Lpf2/LWPConst.hpp"

using DeviceCapabilityId = uint32_t;

namespace Lpf2
{
    class Device
    {
    public:
        Device() {};
        virtual ~Device() = default;

        virtual bool init() = 0;
        virtual void update() = 0;
        virtual const char *name() const = 0;

        virtual bool hasCapability(DeviceCapabilityId id) const = 0;
        virtual void *getCapability(DeviceCapabilityId id) = 0;

        virtual DeviceType getDeviceType() const = 0;
        virtual const std::vector<Mode> &getModes() const = 0;
        virtual std::vector<uint16_t> getModeCombos() const = 0;
        virtual uint8_t getModeCount() const = 0;
        virtual Version getFwVersion() const = 0;
        virtual Version getHwVersion() const = 0;

        /**
         * @returns mode bitmask
         */
        virtual uint16_t getInputModes() const = 0;

        /**
         * @returns mode bitmask
         */
        virtual uint16_t getOutputModes() const = 0;

        virtual uint8_t getCapabilities() const = 0;
        virtual int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) = 0;
        virtual int setMode(uint8_t mode, float delta = 1.0f) = 0;
        virtual int setModeCombo(uint8_t idx, const std::vector<float>& deltas = {}) = 0;
    };
}; // namespace Lpf2