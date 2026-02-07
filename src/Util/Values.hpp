#pragma once
#ifndef _LPF2_UTILS_VALUES_
#define _LPF2_UTILS_VALUES_
#include <string>
#include <vector>
#include <cstdint>
#include "../Lpf2Const.hpp"

namespace Lpf2Utils
{
    std::string bytes_to_hexString(const std::vector<uint8_t> &data);
    std::string byte_to_hexString(uint8_t data);

    std::vector<uint8_t> packVersion(Lpf2Version version);
    Lpf2Version unPackVersion(std::vector<uint8_t> version);
} // namespace Lpf2Utils


#endif