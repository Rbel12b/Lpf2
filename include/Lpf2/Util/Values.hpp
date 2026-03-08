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

#include <string>
#include <vector>
#include <cstdint>
#include "Lpf2/LWPConst.hpp"

namespace Lpf2::Utils
{
    std::string bytes_to_hexString(const std::vector<uint8_t> &data);
    std::string bytes_to_hexString(const std::string &data);
    std::string byte_to_hexString(uint8_t data);

    std::vector<uint8_t> packVersion(Version version);
    Version unPackVersion(std::vector<uint8_t> version);
} // namespace Lpf2Utils