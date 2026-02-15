#pragma once
#ifndef _LPF2_VIRTUAL_DEVICE_H_
#define _LPF2_VIRTUAL_DEVICE_H_

#include "config.hpp"
#include "Lpf2Const.hpp"
#include <vector>
#include "../Util/Values.hpp"
#include "../Lpf2Device.hpp"

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


    /**
     * @brief set motor power
     * @param pw motor power: LPF2_POWER_FLOAT -> floating, LPF2_POWER_BRAKE -> brake,
     * -100 - 0 -> CCW, 0 - 100 -> CW
     */
    virtual void startPower(int8_t pw) = 0;

    /**
     * @brief set acceleration time/profile for a motor
     * @param accTime acceleration time in ms
     * @param accProfile acceleration profile to use, ???, use 1
     */
    virtual void setAccTime(uint16_t accTime, Lpf2AccelerationProfile accProfile = 1) = 0;

    /**
     * @brief set deceleration time/profile for a motor
     * @param decTime acceleration time in ms
     * @param decProfile deceleration profile to use, ???, use 1
     */
    virtual void setDecTime(uint16_t decTime, Lpf2AccelerationProfile decProfile = 1) = 0;

    /**
     * @brief start motor with speed
     * @param speed LPF2_POWER_FLOAT -> hold, LPF2_POWER_BRAKE -> brake,
     * -100 - 0 -> CCW, 0 - 100 -> CW
     * @param maxPower max motor power, absolute value, 0-100%
     * @param useProfile
        0x0000000? -> Acc.- profile
        0x000000?0 -> Decc.- profile
     */
    virtual void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile = 0) = 0;

    /**
     * @brief start motor with speed for [time] ms
     * @param time time in ms
     * @param speed speed: -100 - 0 -> CCW, 0 - 100 -> CW
     * @param maxPower max motor power
     * @param endState what happens after command is finished
     * @param useProfile
        0x0000000? -> Acc.- profile
        0x000000?0 -> Decc.- profile
     */
    virtual void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile = 0) = 0;

    /**
     * @brief start motor with speed for [degrees]°
     * @param degrees degrees to move (positive value, use speed for CCW movement)
     * @param speed speed: -100 - 0 -> CCW, 0 - 100 -> CW
     * @param maxPower max motor power
     * @param endState what happens after command is finished
     * @param useProfile
        0x0000000? -> Acc.- profile
        0x000000?0 -> Decc.- profile
     */
    virtual void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile = 0) = 0;

    /**
     * @brief got to absolute position (motor)
     * @param absPos absolute position
     * @param speed speed: 0 - 100
     * @param maxPower max motor power
     * @param endState what happens after command is finished
     * @param useProfile
        0x0000000? -> Acc.- profile
        0x000000?0 -> Decc.- profile
     */
    virtual void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile = 0) = 0;

    /**
     * @brief preset encoder, also stops motors
     * @param pos position to set the encoder to
     */
    virtual void presetEncoder(int32_t pos) = 0;
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

    void startPower(int8_t pw) override
    {
        LPF2_LOG_I("[%02X] start power %d",
            static_cast<uint8_t>(m_desc.type),
            pw);
    }

    void setAccTime(uint16_t accTime, Lpf2AccelerationProfile accProfile) override
    {
        LPF2_LOG_I("[%02X] set acc time %d profile %d",
            static_cast<uint8_t>(m_desc.type),
            accTime, accProfile);
    }

    void setDecTime(uint16_t decTime, Lpf2AccelerationProfile decProfile) override
    {
        LPF2_LOG_I("[%02X] set dec time %d profile %d",
            static_cast<uint8_t>(m_desc.type),
            decTime, decProfile);
    }

    void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile) override
    {
        LPF2_LOG_I("[%02X] start speed %d max power %d profile %d",
            static_cast<uint8_t>(m_desc.type),
            speed, maxPower, useProfile);
    }

    void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile) override
    {
        LPF2_LOG_I("[%02X] start speed for time %d speed %d max power %d end state %d profile %d",
            static_cast<uint8_t>(m_desc.type),
            time, speed, maxPower, static_cast<uint8_t>(endState), useProfile);
    }

    void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile) override
    {
        LPF2_LOG_I("[%02X] start speed for degrees %d speed %d max power %d end state %d profile %d",
            static_cast<uint8_t>(m_desc.type),
            degrees, speed, maxPower, static_cast<uint8_t>(endState), useProfile);
    }

    void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile) override
    {
        LPF2_LOG_I("[%02X] goto abs position %d speed %d max power %d end state %d profile %d",
            static_cast<uint8_t>(m_desc.type),
            absPos, speed, maxPower, static_cast<uint8_t>(endState), useProfile);
    }

    void presetEncoder(int32_t pos) override
    {
        LPF2_LOG_I("[%02X] preset encoder %d",
            static_cast<uint8_t>(m_desc.type),
            pos);
    }

private:
    const Lpf2DeviceDescriptor& m_desc;
};

#endif