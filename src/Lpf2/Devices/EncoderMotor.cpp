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

#include "Lpf2/Devices/EncoderMotor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"
namespace Lpf2::Devices
{
    namespace EncoderFactory
    {
        EncoderMotorFactory factory = {};
    }

    void EncoderMotor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&EncoderFactory::factory);
    }

    bool EncoderMotor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *EncoderMotor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<EncoderMotorControl *>(this);
        if (id == BasicMotor::CAP)
            return static_cast<BasicMotorControl *>(this);
        return nullptr;
    }

    bool EncoderMotorFactory::matches(const Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::MEDIUM_LINEAR_MOTOR:
        case DeviceType::TECHNIC_LARGE_LINEAR_MOTOR:
        case DeviceType::TECHNIC_XLARGE_LINEAR_MOTOR:
        case DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR:
        case DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR:
        case DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY:
        case DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR_GREY:
            return true;
        default:
            break;
        }
        return false;
    }

    void EncoderMotor::startPower(int8_t pw)
    {
        m_port.startPower(pw);
    }

    void EncoderMotor::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        m_port.setAccTime(accTime, accProfile);
    }

    void EncoderMotor::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        m_port.setDecTime(decTime, decProfile);
    }

    void EncoderMotor::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        m_port.startSpeed(speed, maxPower, useProfile);
    }

    void EncoderMotor::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        m_port.startSpeedForTime(time, speed, maxPower, endState, useProfile);
    }

    void EncoderMotor::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        m_port.startSpeedForDegrees(degrees, speed, maxPower, endState, useProfile);
    }

    void EncoderMotor::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        m_port.gotoAbsPosition(absPos, speed, maxPower, endState, useProfile);
    }

    void EncoderMotor::presetEncoder(int32_t pos)
    {
        m_port.presetEncoder(pos);
    }
}; // namespace Lpf2::Devices