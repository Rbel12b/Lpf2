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