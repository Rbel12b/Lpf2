#pragma once
#ifndef _LPF2_VIRTUAL_DEVICE_H_
#define _LPF2_VIRTUAL_DEVICE_H_

#include "config.h"
#include "Lpf2Const.h"
#include <vector>
#include "../Util/Values.h"
#include "../Lpf2Device.h"

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
    virtual int setMode(uint8_t mode) = 0;
    virtual int setModeCombo(uint8_t idx) = 0;
};

class Lpf2VirtualGenericDevice : public Lpf2VirtualDevice
{
public:
    explicit Lpf2VirtualGenericDevice(const Lpf2DeviceDescriptor& desc)
        : m_desc(desc) {}

    Lpf2DeviceType getDeviceType() const override
    {
        return m_desc.type;
    }

    const std::vector<Lpf2Mode>& getModes() const override
    {
        return m_desc.modes;
    }

    uint8_t getModeCount() const override
    {
        return m_desc.modes.size();
    }

    uint16_t getInputModes() const override
    {
        return m_desc.inModes;
    }

    uint16_t getOutputModes() const override
    {
        return m_desc.outModes;
    }

    uint8_t getCapatibilities() const override
    {
        return m_desc.caps;
    }

    std::vector<uint16_t> getModeCombos() const override
    {
        return m_desc.combos;
    }

    int writeData(uint8_t mode, const std::vector<uint8_t>& data) override
    {
        LPF2_LOG_I("[%02X] write mode %d: %s",
            static_cast<uint8_t>(m_desc.type),
            mode,
            Lpf2Utils::bytes_to_hexString(data).c_str());
        return 0;
    }

    void setPower(uint8_t pin1, uint8_t pin2) override
    {
        LPF2_LOG_I("[%02X] power %d %d",
            static_cast<uint8_t>(m_desc.type),
            pin1, pin2);
    }

    int setMode(uint8_t mode) override
    {
        LPF2_LOG_I("[%02X] set mode %d",
            static_cast<uint8_t>(m_desc.type),
            mode);
        return 0;
    }

    int setModeCombo(uint8_t idx) override
    {
        LPF2_LOG_I("[%02X] set mode combo %d",
            static_cast<uint8_t>(m_desc.type),
            idx);
        return 0;
    }

private:
    const Lpf2DeviceDescriptor& m_desc;
};

#endif