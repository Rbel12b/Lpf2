#pragma once
#ifndef _LPF2_PORT_H_
#define _LPF2_PORT_H_

#include "config.h"
#include "Lpf2Const.h"

#define MEASUREMENTS 20
class Lpf2Port
{
public:
    class Mode
    {
    public:
        std::string name;
        float min = 0.0f, max = 1023.0f;
        float SImin = 0.0f, SImax = 1023.0f;
        bool negativePCT = false;
        std::string unit;
        struct Mapping
        {
            uint8_t val;

            bool nullSupport() const { return val & (1 << 7); }
            bool mapping2() const { return val & (1 << 6); }
            bool m_abs() const { return val & (1 << 4); }
            bool m_rel() const { return val & (1 << 3); }
            bool m_dis() const { return val & (1 << 2); }
        };
        Mapping in, out;
        uint8_t data_sets = 0, format = 0, figures = 0, decimals = 0;
        std::vector<uint8_t> rawData;
        struct Flags
        {
            uint8_t bytes[6] = {0, 0, 0, 0, 0, 0};

            bool speed() const { return bytes[0] & (1 << 0); }
            bool apos() const { return bytes[0] & (1 << 1); }
            bool pos() const { return bytes[0] & (1 << 2); }
            bool power() const { return bytes[0] & (1 << 4); }
            bool motor() const { return bytes[0] & (1 << 5); }
            bool pin1() const { return bytes[0] & (1 << 6); }
            bool pin2() const { return bytes[0] & (1 << 7); }

            bool calib() const { return bytes[1] & (1 << 6); }

            bool power12() const { return bytes[4] & (1 << 0); }
        };
        Flags flags;
    };

    virtual void update() = 0;

    static float getValue(const Mode &modeData, uint8_t dataSet);
    float getValue(uint8_t modeNum, uint8_t dataSet) const;
    static std::string formatValue(float value, const Mode &modeData);
    static std::string convertValue(Mode modeData);
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
    const std::vector<Mode> &getModes() const { return modeData; }
    uint8_t getModeComboCount() const { return comboNum; }

    uint16_t getModeCombo(uint8_t combo) const
    {
        if (combo >= comboNum || combo >= 16)
            return 0;
        return modeCombos[combo];
    }

    virtual bool deviceConnected() = 0;

    std::vector<Mode> modeData;

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
    uint16_t modeCombos[16];
    uint8_t comboNum = 0;
};

#endif