#pragma once
#ifndef _LPF2_PORT_H_
#define _LPF2_PORT_H_

#include "config.h"
#include "Lpf2Const.h"

class Lpf2Port
{
public:
    virtual void update() = 0;

    static float getValue(const Lpf2Mode &modeData, uint8_t dataSet);
    float getValue(uint8_t modeNum, uint8_t dataSet) const;
    static std::string formatValue(float value, const Lpf2Mode &modeData);
    static std::string convertValue(Lpf2Mode modeData);
    std::string convertValue(uint8_t modeNum) const
    {
        if (modeNum >= modeData.size())
        {
            return "<mode not found>";
        }
        return convertValue(modeData[modeNum]);
    }

    virtual int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) = 0;

    virtual void setPower(uint8_t pin1, uint8_t pin2) = 0;

    Lpf2DeviceType getDeviceType() const { return m_deviceType; }
    size_t getSpeed() const { return baud; }
    uint8_t getModeCount() const { return modes; }
    uint8_t getViewCount() const { return views; }
    const std::vector<Lpf2Mode> &getModes() const { return modeData; }
    uint8_t getModeComboCount() const { return comboNum; }

    uint16_t getModeCombo(uint8_t combo) const
    {
        if (combo >= comboNum || combo >= 16)
            return 0;
        return modeCombos[combo];
    }

    /**
     * @returns mode bitmask
     */
    uint16_t getInputModes() const { return inModes; }
    /**
     * @returns mode bitmask
     */
    uint16_t getOutputModes() const { return outModes; }
    uint8_t getCapatibilities() const { return caps; }

    virtual bool deviceConnected() = 0;

    std::vector<Lpf2Mode> modeData;

    Lpf2PortNum portNum; // just a reference for some external code. (e.g. you can set it and use it, but only if Lpf2HubEmulation does not use it!!!)
protected:
    static uint8_t getDataSize(uint8_t format);
    static Lpf2ModeNum getDefaultMode(Lpf2DeviceType id);
    static bool deviceIsAbsMotor(Lpf2DeviceType id);

    /// Parse a signed 8-bit integer from raw bytes
    static float parseData8(const uint8_t *ptr);

    /// Parse a signed 16-bit little-endian integer from raw bytes
    static float parseData16(const uint8_t *ptr);

    /// Parse a signed 32-bit little-endian integer from raw bytes
    static float parseData32(const uint8_t *ptr);

    /// Parse a 32-bit IEEE-754 little-endian float
    static float parseDataF(const uint8_t *ptr);

protected:
    Lpf2DeviceType m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
    uint8_t modes, views;
    uint32_t baud = 2400;
    std::vector<uint16_t> modeCombos;
    uint8_t caps;
    /* bitmask */
    uint16_t inModes, outModes;
    uint8_t comboNum = 0;
};

#endif