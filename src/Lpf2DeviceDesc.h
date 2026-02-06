#pragma once
#ifndef _LPF2_DEVICE_DESCRIPTOR_H_
#define _LPF2_DEVICE_DESCRIPTOR_H_

struct Lpf2DeviceDescriptor
{
    Lpf2DeviceType type;
    uint16_t inModes;
    uint16_t outModes;
    uint8_t caps;
    std::vector<uint16_t> combos;
    std::vector<Lpf2Mode> modes;
};

#endif