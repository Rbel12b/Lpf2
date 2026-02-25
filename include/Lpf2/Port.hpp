#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/LWPConst.hpp"
#include "Lpf2/DeviceDesc.hpp"

namespace Lpf2
{
    class Port
    {
        friend class HubEmulation;
    public:
        virtual void update() = 0;

        virtual int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) = 0;

        /**
         * @brief set motor power
         * @param pw motor power:
         * -100 - 0 -> CCW, 0 - 100 -> CW
         */
        virtual void startPower(int8_t pw) = 0;

        /**
         * @brief set acceleration time/profile for a motor
         * @param accTime acceleration time in ms
         * @param accProfile acceleration profile to use, ???, use 1
         */
        virtual void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) = 0;

        /**
         * @brief set deceleration time/profile for a motor
         * @param decTime acceleration time in ms
         * @param decProfile deceleration profile to use, ???, use 1
         */
        virtual void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) = 0;

        /**
         * @brief start motor with speed
         * @param speed speed:
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
        virtual void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) = 0;

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
        virtual void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) = 0;

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
        virtual void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) = 0;

        /**
         * @brief preset encoder, also stops motors
         * @param pos position to set the encoder to
         */
        virtual void presetEncoder(int32_t pos) = 0;

        /**
         * @brief uses writeData with mode 0 to set the color idx
         * (works with an rgb led - hub led)
         */
        void setRgbColorIdx(ColorIDX idx);

        /**
         * @brief uses writeData with mode 1 to set the color
         * (works with an rgb led - hub led)
         */
        void setRgbColor(uint8_t r, uint8_t g, uint8_t b);

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

        static float getValue(const Mode &modeData, const std::vector<uint8_t> &raw, uint8_t dataSet);
        static float getValue(const Mode &modeData, uint8_t dataSet);
        float getValue(uint8_t modeNum, const std::vector<uint8_t> &raw, uint8_t dataSet)  const;
        float getValue(uint8_t modeNum, uint8_t dataSet) const;
        static std::string formatValue(float value, const Mode &modeData);
        static std::string convertValue(const Mode& modeData);
        std::string convertValue(uint8_t modeNum) const
        {
            if (modeNum >= modeData.size())
            {
                return "<mode not found>";
            }
            return convertValue(modeData[modeNum]);
        }

        DeviceType getDeviceType() const { return m_deviceType; }
        uint8_t getModeCount() const { return modeData.size(); }
        uint8_t getViewCount() const { return views; }
        const std::vector<Mode> &getModes() const { return modeData; }
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

        PortNum getPortNum() const { return portNum; }

        /**
         * @brief Set the port data from a device descriptor,
         * the function will check if the device type matches before setting the data
         * @param desc Device descriptor to set from
         */
        void setFromDesc(const DeviceDescriptor *desc)
        {
            if (!desc || desc->type != m_deviceType)
                return;
            m_rawDataSizeEnsured = false;
            modeData = desc->modes;
            modes = modeData.size();
            views = 0; // TODO: add to desc
            modeCombos = desc->combos;
            caps = desc->caps;
            inModes = desc->inModes;
            outModes = desc->outModes;
        }

        /**
         * @brief convert speed to a 8 bit raw value
         * @param speed -100..100
         */
        uint8_t speedToRaw(int8_t speed)
        {
            uint8_t raw;
            if (speed == 0)
            {
                raw = 127;
            }
            else if (speed > 0)
            {
                raw = map(speed, 0, 100, 0, 126);
            }
            else
            {
                raw = map(-speed, 0, 100, 255, 128);
            }
            return raw;
        }

    protected:
        static uint8_t getDataSize(uint8_t format);
        static ModeNum getDefaultMode(DeviceType id);
        static bool deviceIsAbsMotor(DeviceType id);

        /// Parse a signed 8-bit integer from raw bytes
        static float parseData8(const uint8_t *ptr);

        /// Parse a signed 16-bit little-endian integer from raw bytes
        static float parseData16(const uint8_t *ptr);

        /// Parse a signed 32-bit little-endian integer from raw bytes
        static float parseData32(const uint8_t *ptr);

        /// Parse a 32-bit IEEE-754 little-endian float
        static float parseDataF(const uint8_t *ptr);

        bool m_rawDataSizeEnsured = false;
        void ensureRawDataSize();

    protected:
        DeviceType m_deviceType = DeviceType::UNKNOWNDEVICE;
        uint8_t modes, views;
        std::vector<uint16_t> modeCombos;
        uint8_t caps;
        /* bitmask */
        uint16_t inModes, outModes;
        uint8_t comboNum = 0;

        std::vector<Mode> modeData;

        PortNum portNum;
    };
}; // namespace Lpf2