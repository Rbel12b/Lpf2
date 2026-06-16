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
#include <algorithm>
#include <cmath>

namespace Lpf2::Local
{
    // 100% speed = 1 degree per ms = 1000 deg/s ≈ 167 RPM
    static constexpr float MAX_MOTOR_SPEED = 1.f;
    // stop position control when within 1°
    static constexpr float POSITION_TOLERANCE = 1.0f;

    void Port::updateMotorPID()
    {
        if (!Lpf2::Devices::EncoderFactory::factory.matches(*this))
        {
            // Only track EncoderMotors
            return;
        }
        // Encoder tracking
        int32_t pos = getMotorPos();
        int32_t delta = pos - m_lastMotorPos;
        if (m_activeCombo < 0)
        {
            // CALIB wraps at 360°
            if (delta > 180) delta -= 360;
            else if (delta < -180) delta += 360;
        }
        m_currentRelPos += delta;
        m_lastMotorPos = pos;

        LPF2_LOG_V("Pos: %d, RelPos: %lld, Delta: %d",
                   pos, m_currentRelPos, delta);

        if (m_pidMode == PidMode::NONE)
            return;

        uint64_t now = LPF2_GET_TIME();
        float dt = (float)(now - m_pidLastMs);
        m_pidLastMs = now;
        if (dt <= 0.0f)
            return;

        // SPEED mode: advance moving position setpoint
        if (m_pidMode == PidMode::SPEED)
        {
            if (m_pidEndTime != 0 && now >= m_pidEndTime)
            {
                applyEndState(m_pidEndState);
                return;
            }
            // Accumulate fractional steps to avoid truncation at low speeds
            m_pidTargetFrac += m_pidSpeed * MAX_MOTOR_SPEED / 100.0f * dt;
            int64_t steps = (int64_t)m_pidTargetFrac;
            m_pidTarget += steps;
            m_pidTargetFrac -= (float)steps;
        }

        // POSITION mode: check arrival
        if (m_pidMode == PidMode::POSITION)
        {
            float remaining = (float)(m_pidTarget - m_currentRelPos);
            if (std::abs(remaining) < POSITION_TOLERANCE)
            {
                applyEndState(m_pidEndState);
                return;
            }
        }

        // PID computation (shared by SPEED, POSITION, HOLD)
        float error = (float)(m_pidTarget - m_currentRelPos);
        m_pidIntegral += error * dt;
        m_pidIntegral = std::max(-1000.0f, std::min(1000.0f, m_pidIntegral));

        float derivative = (error - m_pidPrevError) / dt;
        m_pidPrevError = error;

        float output = m_kp * error + m_ki * m_pidIntegral + m_kd * derivative;

        float maxPwr = (float)m_pidMaxPower;
        output = std::max(-maxPwr, std::min(maxPwr, output));

        applyPower((int8_t)std::roundf(output));
    }

    void Port::applyPower(int8_t pw)
    {
        bool forward = pw >= 0;
        pw = std::abs(pw);
        if (pw > 100)
            pw = 100;
        uint8_t pwr2 = pw * 0xFF / 100;
        uint8_t pwr1 = 0;
        if (!forward)
            std::swap(pwr1, pwr2);
        setPower(pwr1, pwr2);
    }

    void Port::applyEndState(BrakingStyle style)
    {
        m_pidIntegral = 0.0f;
        m_pidPrevError = 0.0f;
        m_pidTargetFrac = 0.0f;
        if (style == BrakingStyle::HOLD)
        {
            m_pidMode = PidMode::HOLD;
            m_pidTarget = m_currentRelPos;
            m_pidMaxPower = 100;
            m_pidEndTime = 0;
            m_pidLastMs = LPF2_GET_TIME();
        }
        else
        {
            m_pidMode = PidMode::NONE;
            if (style == BrakingStyle::BRAKE)
                setPower(0xFF, 0xFF);
            else
                setPower(0, 0); // FLOAT
        }
    }

    void Port::startPower(int8_t pw)
    {
        m_pidMode = PidMode::NONE;
        bool forward = pw >= 0;
        pw = std::abs(pw);
        if (pw > 100)
            pw = 100;

        uint8_t pwr2 = pw * 0xFF / 100;
        uint8_t pwr1 = 0;
        if (!forward)
            std::swap(pwr1, pwr2);

        LPF2_LOG_V("startPower: %i -> %i, %i", (int8_t)pw, pwr1, pwr2);
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
        if (m_pidMode == PidMode::SPEED && m_pidSpeed == speed && m_pidMaxPower == maxPower && m_pidEndTime == 0)
            return;
        m_pidSpeed = speed;
        m_pidMaxPower = maxPower;
        m_pidTarget = m_currentRelPos;
        m_pidTargetFrac = 0.0f;
        m_pidEndTime = 0;
        m_pidIntegral = 0.0f;
        m_pidPrevError = 0.0f;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::SPEED;
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        uint64_t newEndTime = LPF2_GET_TIME() + time;
        if (m_pidMode == PidMode::SPEED && m_pidSpeed == speed && m_pidMaxPower == maxPower && m_pidEndState == endState)
        {
            m_pidEndTime = newEndTime;
            return;
        }
        m_pidSpeed = speed;
        m_pidMaxPower = maxPower;
        m_pidTarget = m_currentRelPos;
        m_pidTargetFrac = 0.0f;
        m_pidEndTime = newEndTime;
        m_pidEndState = endState;
        m_pidIntegral = 0.0f;
        m_pidPrevError = 0.0f;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::SPEED;
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        int64_t signedDeg = (int64_t)degrees * (speed >= 0 ? 1 : -1);
        int64_t newTarget = m_currentRelPos + signedDeg;
        if (m_pidMode == PidMode::POSITION && m_pidTarget == newTarget && m_pidMaxPower == maxPower && m_pidEndState == endState && m_pidEndTime == 0)
            return;
        m_pidTarget = newTarget;
        m_pidMaxPower = maxPower;
        m_pidEndState = endState;
        m_pidEndTime = 0;
        m_pidTargetFrac = 0.0f;
        m_pidIntegral = 0.0f;
        m_pidPrevError = 0.0f;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::POSITION;
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        int64_t newTarget = (int64_t)absPos;
        uint8_t newMaxPower = std::min(speed, maxPower);
        if (m_pidMode == PidMode::POSITION && m_pidTarget == newTarget && m_pidMaxPower == newMaxPower && m_pidEndState == endState && m_pidEndTime == 0)
            return;
        m_pidTarget = newTarget;
        m_pidMaxPower = newMaxPower;
        m_pidEndState = endState;
        m_pidEndTime = 0;
        m_pidTargetFrac = 0.0f;
        m_pidIntegral = 0.0f;
        m_pidPrevError = 0.0f;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::POSITION;
    }

    void Port::presetEncoder(int32_t pos)
    {
        m_currentRelPos = (int64_t)pos;
        m_pidMode = PidMode::NONE;
        setPower(0, 0);
    }
}; // namespace Lpf2::Local
