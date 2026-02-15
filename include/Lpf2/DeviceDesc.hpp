#pragma once

#include "Lpf2/LWPConst.hpp"

namespace Lpf2
{
    struct DeviceDescriptor
    {
        DeviceType type;
        uint16_t inModes;
        uint16_t outModes;
        uint8_t caps;
        std::vector<uint16_t> combos;
        std::vector<Mode> modes;
    };
}; // namespace Lpf2