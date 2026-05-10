/**
 *  Copyright (C) 2026 - Rbel12b
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  */

#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/LWPConst.hpp"
#include "Lpf2/Util/Values.hpp"
#include "Lpf2/Device.hpp"
#include "Lpf2/DeviceFactory.hpp"

namespace Lpf2::Virtual
{
    class Device : public Lpf2::Device
    {
    public:
        /**
         * @brief set the calbback that will be called on mode data change
         */
        void setValueChangeCallback(ValueChangeCallback callback)
        {
            m_valueChangeCallback = callback;
        }

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
        virtual void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) = 0;

        /**
         * @brief set deceleration time/profile for a motor
         * @param decTime acceleration time in ms
         * @param decProfile deceleration profile to use, ???, use 1
         */
        virtual void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) = 0;

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

    protected:
        ValueChangeCallback m_valueChangeCallback = nullptr;
    };

    class GenericDevice : public Virtual::Device
    {
    public:
        explicit GenericDevice(const DeviceDescriptor &desc)
            : m_desc(desc), m_modes(m_desc.modes) {}

        bool init() override { return true; }
        void update() override {}
        const char *name() const override { return "Lpf2::Virtual::GenericDevice"; }

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("virtual_device_generic");

        bool hasCapability(DeviceCapabilityId id) const override
        {
            return id == CAP;
        }

        void *getCapability(DeviceCapabilityId id) override
        {
            return (id == CAP ? this : nullptr);
        }

        DeviceType getDeviceType() const override
        {
            return m_desc.type;
        }

        const std::vector<Mode> &getModes() const override
        {
            return m_modes;
        }

        uint8_t getModeCount() const override
        {
            return m_modes.size();
        }

        uint16_t getInputModes() const override
        {
            return m_desc.inModesMask;
        }

        uint16_t getOutputModes() const override
        {
            return m_desc.outModesMask;
        }

        uint8_t getCapabilities() const override
        {
            return m_desc.caps;
        }

        std::vector<uint16_t> getModeCombos() const override
        {
            return m_desc.combos;
        }

        Version getFwVersion() const override
        {
            return m_desc.fwVersion;
        }

        Version getHwVersion() const override
        {
            return m_desc.hwVersion;
        }

        void setUserData(void *data)
        {
            m_userData = data;
        }

        void setModeData(uint8_t modeNum, const std::vector<uint8_t> &data)
        {
            if (modeNum < m_modes.size())
            {
                m_modes[modeNum].rawData = data;
                if (m_valueChangeCallback)
                {
                    m_valueChangeCallback(modeNum);
                }
            }
        }

        using WriteDataCallback = std::function<int(uint8_t mode, const std::vector<uint8_t> &data, void *userData)>;

        void setWriteDataCallback(WriteDataCallback callback)
        {
            m_writeDataCallback = callback;
        };

        virtual int writeData(uint8_t mode, const std::vector<uint8_t> &data) override
        {
            if (m_writeDataCallback)
            {
                return m_writeDataCallback(mode, data, m_userData);
            }

            LPF2_LOG_I("[%02X] write mode %d: %s",
                       static_cast<uint8_t>(m_desc.type),
                       mode,
                       Lpf2::Utils::bytes_to_hexString(data).c_str());
            return 0;
        }

        virtual int setMode(uint8_t mode) override
        {
            LPF2_LOG_I("[%02X] set mode %d",
                       static_cast<uint8_t>(m_desc.type),
                       mode);
            return 0;
        }

        virtual int setModeCombo(uint8_t idx) override
        {
            LPF2_LOG_I("[%02X] set mode combo %d",
                       static_cast<uint8_t>(m_desc.type),
                       idx);
            return 0;
        }

        virtual void startPower(int8_t pw) override
        {
            LPF2_LOG_I("[%02X] start power %d",
                       static_cast<uint8_t>(m_desc.type),
                       pw);
        }

        virtual void setAccTime(uint16_t accTime, AccelerationProfile accProfile) override
        {
            LPF2_LOG_I("[%02X] set acc time %d profile %d",
                       static_cast<uint8_t>(m_desc.type),
                       accTime, accProfile);
        }

        virtual void setDecTime(uint16_t decTime, AccelerationProfile decProfile) override
        {
            LPF2_LOG_I("[%02X] set dec time %d profile %d",
                       static_cast<uint8_t>(m_desc.type),
                       decTime, decProfile);
        }

        virtual void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile) override
        {
            LPF2_LOG_I("[%02X] start speed %d max power %d profile %d",
                       static_cast<uint8_t>(m_desc.type),
                       speed, maxPower, useProfile);
        }

        virtual void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile) override
        {
            LPF2_LOG_I("[%02X] start speed for time %d speed %d max power %d end state %d profile %d",
                       static_cast<uint8_t>(m_desc.type),
                       time, speed, maxPower, static_cast<uint8_t>(endState), useProfile);
        }

        virtual void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile) override
        {
            LPF2_LOG_I("[%02X] start speed for degrees %d speed %d max power %d end state %d profile %d",
                       static_cast<uint8_t>(m_desc.type),
                       degrees, speed, maxPower, static_cast<uint8_t>(endState), useProfile);
        }

        virtual void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile) override
        {
            LPF2_LOG_I("[%02X] goto abs position %d speed %d max power %d end state %d profile %d",
                       static_cast<uint8_t>(m_desc.type),
                       absPos, speed, maxPower, static_cast<uint8_t>(endState), useProfile);
        }

        virtual void presetEncoder(int32_t pos) override
        {
            LPF2_LOG_I("[%02X] preset encoder %d",
                       static_cast<uint8_t>(m_desc.type),
                       pos);
        }

    private:
        const DeviceDescriptor &m_desc;
        std::vector<Mode> m_modes;

        void *m_userData = nullptr;
        WriteDataCallback m_writeDataCallback = nullptr;
    };
}; // namespace Lpf2