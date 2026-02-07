#pragma once
#ifndef _LPF2_PORT_H_
#define _LPF2_PORT_H_

#include "config.h"
#include "Lpf2Const.h"
#include "Lpf2DeviceDesc.h"

class Lpf2Port
{
public:
    virtual void update() = 0;

    virtual int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) = 0;

    virtual void setPower(uint8_t pin1, uint8_t pin2) = 0;

    /**
     * @brief Select the mode of the connected device,
     * has no effect when deviceConnected() == false
     * @returns 0 if succesful
     */
    virtual int setMode(uint8_t mode) = 0;

    /**
     * @brief Set the mode combination of the connected device,
     * has no effect when deviceConnected() == false
     * @param idx the mode combo index (from getModeCombos())
     * @returns 0 if succesful
     */
    virtual int setModeCombo(uint8_t idx) = 0;

    virtual bool deviceConnected() = 0;

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

    Lpf2DeviceType getDeviceType() const { return m_deviceType; }
    uint8_t getModeCount() const { return modeData.size(); }
    uint8_t getViewCount() const { return views; }
    const std::vector<Lpf2Mode> &getModes() const { return modeData; }
    uint8_t getModeComboCount() const { return modeCombos.size(); }
    std::vector<uint16_t> getModeCombos() const { return modeCombos; }

    uint16_t getModeCombo(uint8_t combo) const
    {
        if (combo >= modeCombos.size() || combo >= 16)
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

    std::string getInfoStr();

    Lpf2PortNum getPortNum() const { return portNum; }

    /**
     * @brief Set the port data from a device descriptor,
     * the function will check if the device type matches before setting the data
     * @param desc Device descriptor to set from
     */
    void setFromDesc(const Lpf2DeviceDescriptor *desc)
    {
        if (!desc || desc->type != m_deviceType)
            return;
        modeData = desc->modes;
        modes = modeData.size();
        views = 0; // TODO: add to desc
        modeCombos = desc->combos;
        caps = desc->caps;
        inModes = desc->inModes;
        outModes = desc->outModes;
    }

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
    std::vector<uint16_t> modeCombos;
    uint8_t caps;
    /* bitmask */
    uint16_t inModes, outModes;
    uint8_t comboNum = 0;

    std::vector<Lpf2Mode> modeData;

    Lpf2PortNum portNum;
};

#endif