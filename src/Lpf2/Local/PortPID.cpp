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

#include "Lpf2/Local/Port.hpp"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/Devices/EncoderMotor.hpp"

namespace Lpf2::Local
{
    void Port::updateMotorPID()
    {
        if (!Lpf2::Devices::EncoderFactory::factory.matches(*this))
        {
            // Only track EncoderMotors
            return;
        }
        // Encoder tracking
        uint16_t absPos = getAbsPos();
        int16_t delta = static_cast<int16_t>(absPos) -
                        static_cast<int16_t>(m_lastAbsPos);

        if (delta > 1800)
            delta -= 3600;
        else if (delta < -1800)
            delta += 3600;

        m_currentRelPos += delta;
        m_lastAbsPos = absPos;

        LPF2_LOG_V("AbsPos: %u, RelPos: %lld, Delta: %d",
                   absPos, m_currentRelPos, delta);
    }

    void Port::startPower(int8_t pw)
    {
        bool forward = pw >= 0;
        pw = std::abs(pw);
        if (pw > 100)
            pw = 100;

        uint8_t pwr2 = pw * 0xFF / 100;
        uint8_t pwr1 = 0;
        if (!forward)
            std::swap(pwr1, pwr2);

        LPF2_LOG_D("startPower: %i -> %i, %i", (int8_t)pw, pwr1, pwr2);
        setPower(pwr1, pwr2);
    }

    void Port::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        // TODO: implement this
    }

    void Port::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        // TODO: implement this
    }

    void Port::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        // TODO: implement this
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        // TODO: implement this
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        // TODO: implement this
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        // TODO: implement this
    }

    void Port::presetEncoder(int32_t pos)
    {
        m_currentRelPos = pos;
    }
}; // namespace Lpf2::Local