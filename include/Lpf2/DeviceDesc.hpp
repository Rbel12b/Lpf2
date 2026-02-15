#pragma once

struct Lpf2DeviceDescriptor
{
    Lpf2DeviceType type;
    uint16_t inModes;
    uint16_t outModes;
    uint8_t caps;
    std::vector<uint16_t> combos;
    std::vector<Lpf2Mode> modes;
};