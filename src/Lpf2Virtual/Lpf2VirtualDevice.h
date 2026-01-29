#pragma once
#ifndef _LPF2_VIRTUAL_DEVICE_H_
#define _LPF2_VIRTUAL_DEVICE_H_

#include "config.h"
#include "Lpf2Const.h"
#include <vector>

class Lpf2VirtualDevice
{
public:
    virtual Lpf2DeviceType getDeviceType() const = 0;
    virtual const std::vector<Lpf2Mode>& getModes() const = 0;
    virtual std::vector<uint16_t> getModeCombos() const = 0;
    virtual uint8_t getModeCount() const = 0;
    /**
     * @returns mode bitmask
     */
    virtual uint16_t getInputModes() const = 0;
    /**
     * @returns mode bitmask
     */
    virtual uint16_t getOutputModes() const = 0;
    virtual uint8_t getCapatibilities() const = 0;
    virtual int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) = 0;
    virtual void setPower(uint8_t pin1, uint8_t pin2) = 0;
};

#endif