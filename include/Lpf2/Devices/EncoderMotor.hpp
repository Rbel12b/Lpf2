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
#include "Lpf2/Device.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"

namespace Lpf2::Devices
{
    class EncoderMotorControl
    {
    public:
        virtual ~EncoderMotorControl() = default;

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
        virtual void startSpeed(int8_t speed, uint8_t maxPower = 100, uint8_t useProfile = 0) = 0;

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
        virtual void startSpeedForTime(uint16_t time, int8_t speed = 100, uint8_t maxPower = 100, BrakingStyle endState = BrakingStyle::HOLD, uint8_t useProfile = 0) = 0;

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
        virtual void startSpeedForDegrees(uint32_t degrees, int8_t speed = 100, uint8_t maxPower = 100, BrakingStyle endState = BrakingStyle::HOLD, uint8_t useProfile = 0) = 0;

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
        virtual void gotoAbsPosition(int32_t absPos, uint8_t speed = 100, uint8_t maxPower = 100, BrakingStyle endState = BrakingStyle::HOLD, uint8_t useProfile = 0) = 0;

        /**
         * @brief preset encoder, also stops motors
         * @param pos position to set the encoder to
         */
        virtual void presetEncoder(int32_t pos) = 0;
    };

    class EncoderMotor : public PortDevice, public EncoderMotorControl, public BasicMotorControl
    {
    public:
        EncoderMotor(Port &port) : PortDevice(port) {}

        bool init() override
        {
            startPower(0);
            m_port.setModeCombo(0);
            return true;
        }

        void update() override {};

        const char *name() const override
        {
            return "Motor with Encoder";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("encoder_motor");

        static void registerFactory(DeviceRegistry &reg);

        /**
         * @brief set motor power
         * @param pw motor power: LPF2_POWER_FLOAT -> floating, LPF2_POWER_BRAKE -> brake,
         * -100 - 0 -> CCW, 0 - 100 -> CW
         */
        void startPower(int8_t pw) override;

        /**
         * @brief set acceleration time/profile for a motor
         * @param accTime acceleration time in ms
         * @param accProfile acceleration profile to use, ???, use 1
         */
        void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) override;

        /**
         * @brief set deceleration time/profile for a motor
         * @param decTime acceleration time in ms
         * @param decProfile deceleration profile to use, ???, use 1
         */
        void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) override;

        /**
         * @brief start motor with speed
         * @param speed LPF2_POWER_FLOAT -> hold, LPF2_POWER_BRAKE -> brake,
         * -100 - 0 -> CCW, 0 - 100 -> CW
         * @param maxPower max motor power, absolute value, 0-100%
         * @param useProfile
            0x0000000? -> Acc.- profile
            0x000000?0 -> Decc.- profile
         */
        void startSpeed(int8_t speed, uint8_t maxPower = 100, uint8_t useProfile = 0) override;

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
        void startSpeedForTime(uint16_t time, int8_t speed = 100, uint8_t maxPower = 100, BrakingStyle endState = BrakingStyle::HOLD, uint8_t useProfile = 0) override;

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
        void startSpeedForDegrees(uint32_t degrees, int8_t speed = 100, uint8_t maxPower = 100, BrakingStyle endState = BrakingStyle::HOLD, uint8_t useProfile = 0) override;

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
        void gotoAbsPosition(int32_t absPos, uint8_t speed = 100, uint8_t maxPower = 100, BrakingStyle endState = BrakingStyle::HOLD, uint8_t useProfile = 0) override;

        /**
         * @brief preset encoder, also stops motors
         * @param pos position to set the encoder to
         */
        void presetEncoder(int32_t pos) override;
    };

    class EncoderMotorFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new EncoderMotor(port);
        }

        const char *name() const
        {
            return "Encoder Motor Factory";
        }
    };

    namespace EncoderFactory
    {
        inline EncoderMotorFactory factory;
    }
}; // namespace Lpf2::Devices