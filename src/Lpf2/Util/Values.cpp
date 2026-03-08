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

#include "Lpf2/log/log.h"
#include "Lpf2/Util/Values.hpp"
#include <sstream>
#include <iomanip>

namespace Lpf2::Utils
{
    std::string bytes_to_hexString(const std::vector<uint8_t> &data)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < data.size(); ++i)
        {
            if (i > 0)
                oss << ", ";
            oss << "0x"
                << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(data[i]);
        }
        return oss.str();
    }

    std::string bytes_to_hexString(const std::string &data)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < data.size(); ++i)
        {
            if (i > 0)
                oss << ", ";
            oss << "0x"
                << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(data[i]);
        }
        return oss.str();
    }

    std::string byte_to_hexString(uint8_t data)
    {
        std::ostringstream oss;
        oss << "0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data);
        return oss.str();
    }

    std::vector<uint8_t> packVersion(Version version)
    {
        std::vector<uint8_t> v;
        v.push_back(version.Build);
        v.push_back(version.Build >> 8);
        v.push_back(version.Bugfix);
        v.push_back(version.Major << 4 | version.Minor);
        return v;
    }

    Version unPackVersion(std::vector<uint8_t> version)
    {
        if (version.size() < 4)
            return Version();

        Version v;
        v.Build = version[0] | (version[1] << 8);
        v.Bugfix = version[2];
        v.Minor = version[3] & 0x0F;
        v.Major = (version[3] >> 4) & 0x0F;
        return v;
    }

} // namespace Lpf2Utils