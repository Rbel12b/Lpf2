#include "Values.hpp"
#include <sstream>
#include <iomanip>
#include "../log/log.h"

namespace Lpf2Utils
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

    std::vector<uint8_t> packVersion(Lpf2Version version)
    {
        std::vector<uint8_t> v;
        v.push_back(version.Build);
        v.push_back(version.Build >> 8);
        v.push_back(version.Bugfix);
        v.push_back(version.Major << 4 | version.Minor);
        return v;
    }

    Lpf2Version unPackVersion(std::vector<uint8_t> version)
    {
        if (version.size() < 4)
            return Lpf2Version();

        Lpf2Version v;
        v.Build = version[0] | (version[1] << 8);
        v.Bugfix = version[2];
        v.Minor = version[3] & 0x0F;
        v.Major = (version[3] >> 4) & 0x0F;
        return v;
    }

} // namespace Lpf2Utils