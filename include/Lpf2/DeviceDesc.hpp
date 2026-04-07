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

#include "Lpf2/LWPConst.hpp"

namespace Lpf2
{
    struct DeviceDescriptor
    {
        DeviceType type = DeviceType::UNKNOWNDEVICE;
        uint16_t inModesMask = 0;
        uint16_t outModesMask = 0;
        uint8_t caps = 0;
        std::vector<uint16_t> combos;
        std::vector<Mode> modes;
    };
}; // namespace Lpf2