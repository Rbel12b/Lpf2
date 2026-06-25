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
#include "Lpf2/Battery.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

// Per-motor PID tuning. All constants live in this file; layout in
// Local/Port.hpp::MotorSettings.

namespace Lpf2::Local
{
    // Position-arrival tolerance for POSITION mode (mdeg).
    static constexpr int64_t POSITION_TOLERANCE_MDEG = 300;

    // Stiction-stuck speed threshold (mdeg/s). Below this the motor is
    // considered stationary and the breakaway kick is applied.
    static constexpr int32_t STUCK_SPEED_MDPS = 100; // 0.1 deg/s

    // ---- Per-motor settings table -------------------------------------------
    //
    // Tune each motor independently. Calibration procedure documented in
    // docs/motor-tuning.md.

    MotorSettings MS_MEDIUM_LINEAR_MOTOR = {
        .id                = DeviceType::MEDIUM_LINEAR_MOTOR,
        .rated_max_speed   = 1200,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 1.5f,
        .speed_ksi         = 10.0f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 10.0f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.2f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.3f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 32.0f,
        .kinetic_floor_pct = 21.0f,
    };
    MotorSettings MS_TECHNIC_LARGE_LINEAR_MOTOR = {
        .id                = DeviceType::TECHNIC_LARGE_LINEAR_MOTOR,
        .rated_max_speed   = 1500,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 0.2f,
        .speed_ksi         = 2.0f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 2.0f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.1f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.3f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 21.0f,
        .kinetic_floor_pct = 17.0f,
    };
    MotorSettings MS_TECHNIC_XLARGE_LINEAR_MOTOR = {
        .id                = DeviceType::TECHNIC_XLARGE_LINEAR_MOTOR,
        .rated_max_speed   = 1500,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 0.5f,
        .speed_ksi         = 3.0f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 1.5f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.2f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.3f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 30.0f,
        .kinetic_floor_pct = 16.0f,
    };
    MotorSettings MS_TECHNIC_MEDIUM_ANGULAR_MOTOR = {
        .id                = DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR,
        .rated_max_speed   = 1000,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 0.2f,
        .speed_ksi         = 3.0f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 2.0f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.1f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.5f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 38.0f,
        .kinetic_floor_pct = 20.0f,
    };
    MotorSettings MS_TECHNIC_LARGE_ANGULAR_MOTOR = {
        .id                = DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR,
        .rated_max_speed   = 1000,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 1.5f,
        .speed_ksi         = 0.8f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 2.0f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.5f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.3f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 26.0f,
        .kinetic_floor_pct = 18.0f,
    };
    MotorSettings MS_TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY = {
        .id                = DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY,
        .rated_max_speed   = 1000,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 0.2f,
        .speed_ksi         = 3.0f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 2.0f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.1f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.5f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 38.0f,
        .kinetic_floor_pct = 20.0f,
    };
    MotorSettings MS_TECHNIC_LARGE_ANGULAR_MOTOR_GREY = {
        .id                = DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR_GREY,
        .rated_max_speed   = 1000,
        .max_voltage_mv    = 9000,
        .speed_ksp         = 1.5f,
        .speed_ksi         = 0.8f,
        .speed_int_clamp   = 100.0f,
        .speed_deadband_pct= 1.0f,
        .pos_kp            = 2.0f,
        .pos_ki            = 0.0f,
        .pos_kd            = 0.5f,
        .pos_int_clamp     = 200.0f,
        .pos_deadband_deg  = 0.3f,
        .pos_decel_mdps2   = 3.0e6,
        .pos_handoff_deg   = 90.0f,
        .breakaway_pct     = 26.0f,
        .kinetic_floor_pct = 18.0f,
    };

    static MotorSettings *SETTINGS_TABLE[] = {
        &MS_MEDIUM_LINEAR_MOTOR,
        &MS_TECHNIC_LARGE_LINEAR_MOTOR,
        &MS_TECHNIC_XLARGE_LINEAR_MOTOR,
        &MS_TECHNIC_MEDIUM_ANGULAR_MOTOR,
        &MS_TECHNIC_LARGE_ANGULAR_MOTOR,
        &MS_TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY,
        &MS_TECHNIC_LARGE_ANGULAR_MOTOR_GREY,
    };

    static const MotorSettings *lookupSettings(DeviceType id)
    {
        for (MotorSettings *s : SETTINGS_TABLE)
        {
            if (s->id == id)
                return s;
        }
        return nullptr;
    }

    // Stiction + kinetic-floor compensation. Returns adjusted power.
    static float applyFrictionComp(float pid_out,
                                   float err_for_sign,
                                   int32_t speed_mdps,
                                   const MotorSettings &s)
    {
        if (std::abs(err_for_sign) <= 0.0f)
            return pid_out;
        float sign_err = (err_for_sign > 0) ? 1.0f : -1.0f;
        if (std::abs(speed_mdps) < STUCK_SPEED_MDPS &&
            std::abs(pid_out) < s.breakaway_pct)
        {
            return s.breakaway_pct * sign_err;
        }
        if (pid_out != 0.0f && std::abs(pid_out) < s.kinetic_floor_pct)
        {
            float sign_out = (pid_out > 0) ? 1.0f : -1.0f;
            return s.kinetic_floor_pct * sign_out;
        }
        return pid_out;
    }

    // ---- Observer -----------------------------------------------------------
    //
    // Encoder-driven: angle = measured, speed = first difference with IIR
    // low-pass. Avoids voltage-feedforward instability of the prior model-
    // based prediction.

    // IIR coefficient: new_estimate = (prev * (N-1) + new_sample) / N.
    static constexpr int32_t SPEED_IIR_N = 4;

    static void observerStep(int64_t measured_mdeg,
                             int32_t dt_ms,
                             int64_t &angleMdeg,
                             int32_t &speedMdegps,
                             int64_t &prevMeasMdeg,
                             bool &prevValid)
    {
        if (dt_ms <= 0)
            return;

        if (prevValid)
        {
            int64_t delta = measured_mdeg - prevMeasMdeg;
            int32_t inst_speed = (int32_t)(delta * 1000 / dt_ms);
            speedMdegps = (speedMdegps * (SPEED_IIR_N - 1) + inst_speed) /
                          SPEED_IIR_N;
        }
        angleMdeg = measured_mdeg;
        prevMeasMdeg = measured_mdeg;
        prevValid = true;
    }

    // ---- updateMotorPID -----------------------------------------------------

    void Port::updateMotorPID()
    {
        DeviceType dt = getDeviceType();
        if (m_settings == nullptr || m_settings->id != dt)
        {
            m_settings = lookupSettings(dt);
            m_obsInit = false;
            m_pidIntegral = 0.0;
            m_pidMode = PidMode::NONE;
            m_pidPrevPosErrValid = false;
            m_pidPosFineActive = false;
        }
        if (m_settings == nullptr)
            return;
        const MotorSettings &s = *m_settings;

        // Encoder tracking.
        int32_t pos = getMotorPos();
        int32_t delta = pos - m_lastMotorPos;
        if (m_activeCombo < 0)
        {
            if (delta > 180) delta -= 360;
            else if (delta < -180) delta += 360;
        }
        m_currentRelPos += delta;
        m_lastMotorPos = pos;

        int64_t measured_mdeg = m_currentRelPos * 1000;

        uint64_t now = LPF2_GET_TIME();
        int32_t dt_ms = (int32_t)(now - m_pidLastMs);
        m_pidLastMs = now;

        if (!m_obsInit)
        {
            m_obsAngleMdeg = measured_mdeg;
            m_obsSpeedMdegps = 0;
            m_obsPrevMeasMdeg = measured_mdeg;
            m_obsPrevValid = false;
            m_lastVoltageMv = 0;
            m_obsInit = true;
            return;
        }

        if (dt_ms <= 0)
            return;

        observerStep(measured_mdeg, dt_ms,
                     m_obsAngleMdeg, m_obsSpeedMdegps,
                     m_obsPrevMeasMdeg, m_obsPrevValid);

        // Prefer motor's self-reported SPEED (mode 1, % of rated, int8).
        float reported_pct = 0.0f;
        if (m_activeCombo >= 0)
        {
            reported_pct = getValue(1, 0);
            m_obsSpeedMdegps =
                (int32_t)(reported_pct * (float)s.rated_max_speed * 10.0f);
        }
        else if (s.rated_max_speed > 0)
        {
            reported_pct = (float)m_obsSpeedMdegps /
                           ((float)s.rated_max_speed * 10.0f);
        }

        LPF2_LOG_V("Pos:%d Rel:%lld ObsA:%lld ObsV:%d",
                   pos, (long long)m_currentRelPos,
                   (long long)m_obsAngleMdeg, m_obsSpeedMdegps);

        if (m_pidMode == PidMode::NONE)
            return;

        int32_t maxPwr = (int32_t)m_pidMaxPower;
        float power_f = 0.0f;
        float err_for_sign = 0.0f;

        if (m_pidMode == PidMode::SPEED)
        {
            if (m_pidEndTime != 0 && now >= m_pidEndTime)
            {
                applyEndState(m_pidEndState);
                return;
            }
            float setpoint_pct = (float)m_pidSpeed;
            float err_pct = setpoint_pct - reported_pct;
            err_for_sign = err_pct;
            if (std::abs(err_pct) < s.speed_deadband_pct)
                err_pct = 0.0f;

            m_pidIntegral += (double)err_pct * (double)dt_ms / 1000.0;
            if (m_pidIntegral > s.speed_int_clamp)
                m_pidIntegral = s.speed_int_clamp;
            if (m_pidIntegral < -s.speed_int_clamp)
                m_pidIntegral = -s.speed_int_clamp;

            float fb_power =
                s.speed_ksp * err_pct + s.speed_ksi * (float)m_pidIntegral;
            float ff_power = (float)m_pidSpeed;
            power_f = ff_power + fb_power;

            m_pidSpeedSetpointMdegps =
                (int32_t)m_pidSpeed * s.rated_max_speed * 10;
        }
        else
        {
            // POSITION / HOLD: trapezoidal ramp + dual sub-mode controller.
            //   Far  → speed sub-mode tracking ramp velocity (P+I on speed).
            //   Near → fine pos sub-mode (P+I + true D on pos error).
            int64_t remaining_ramp = m_pidPositionFinal - m_pidTarget;
            int32_t step_dir = (remaining_ramp > 0) ? 1 :
                               (remaining_ramp < 0) ? -1 : 0;

            int64_t abs_rem = std::abs(remaining_ramp);
            int32_t v_cap_mdps =
                (int32_t)std::sqrt(2.0 * s.pos_decel_mdps2 * (double)abs_rem);
            int32_t cur_ramp =
                std::min(m_pidPositionRampMdegps, v_cap_mdps);

            int64_t step_max = (int64_t)cur_ramp * dt_ms / 1000;
            int64_t step =
                (abs_rem < (uint64_t)step_max) ? remaining_ramp
                                                : step_dir * step_max;
            m_pidTarget += step;
            m_pidSpeedSetpointMdegps =
                (step_dir != 0) ? (step_dir * cur_ramp) : 0;

            int64_t remaining_real = m_pidPositionFinal - m_obsAngleMdeg;
            float rem_deg = (float)((double)remaining_real / 1000.0);

            if (m_pidMode == PidMode::POSITION)
            {
                if (std::abs(remaining_real) < POSITION_TOLERANCE_MDEG &&
                    std::abs(m_obsSpeedMdegps) < 50000 &&
                    step_dir == 0)
                {
                    applyEndState(m_pidEndState);
                    return;
                }
            }

            // Sub-mode selection. Fine engages when ramp is finished OR
            // we are inside the handoff band. step_dir==0 is a one-way
            // gate (ramp can't restart) so no hysteresis needed.
            bool wantFine = (step_dir == 0) ||
                            (std::abs(rem_deg) <= s.pos_handoff_deg);

            if (wantFine != m_pidPosFineActive)
            {
                m_pidIntegral = 0.0;
                m_pidPrevPosErrValid = false;
                m_pidPosFineActive = wantFine;
            }

            if (!wantFine)
            {
                // Coarse sub-mode: speed loop tracking ramp velocity.
                float setpoint_pct =
                    (s.rated_max_speed > 0)
                        ? ((float)m_pidSpeedSetpointMdegps /
                           (s.rated_max_speed * 10.0f))
                        : 0.0f;
                float err_pct = setpoint_pct - reported_pct;
                err_for_sign = rem_deg;
                if (std::abs(err_pct) < s.speed_deadband_pct)
                    err_pct = 0.0f;

                m_pidIntegral += (double)err_pct * (double)dt_ms / 1000.0;
                if (m_pidIntegral > s.speed_int_clamp)
                    m_pidIntegral = s.speed_int_clamp;
                if (m_pidIntegral < -s.speed_int_clamp)
                    m_pidIntegral = -s.speed_int_clamp;

                float fb_power = s.speed_ksp * err_pct
                               + s.speed_ksi * (float)m_pidIntegral;
                float ff_power = setpoint_pct;
                power_f = ff_power + fb_power;
            }
            else
            {
                // Fine sub-mode: P + I + true D on pos error.
                float pos_err_deg = rem_deg;
                err_for_sign = pos_err_deg;
                if (std::abs(pos_err_deg) < s.pos_deadband_deg)
                    pos_err_deg = 0.0f;

                float d_err_dps = 0.0f;
                if (m_pidPrevPosErrValid && dt_ms > 0)
                    d_err_dps = (pos_err_deg - m_pidPrevPosErrDeg) *
                                1000.0f / (float)dt_ms;
                m_pidPrevPosErrDeg = pos_err_deg;
                m_pidPrevPosErrValid = true;

                m_pidIntegral += (double)pos_err_deg * (double)dt_ms / 1000.0;
                if (m_pidIntegral > s.pos_int_clamp)
                    m_pidIntegral = s.pos_int_clamp;
                if (m_pidIntegral < -s.pos_int_clamp)
                    m_pidIntegral = -s.pos_int_clamp;

                power_f = s.pos_kp * pos_err_deg
                        + s.pos_ki * (float)m_pidIntegral
                        + s.pos_kd * d_err_dps;
            }
        }

        // Stiction kick + kinetic floor.
        power_f = applyFrictionComp(power_f, err_for_sign,
                                    m_obsSpeedMdegps, s);

        int32_t power_pct = (int32_t)power_f;

        // Over-voltage cap: when battery > motor nameplate, limit PWM so the
        // motor never sees above s.max_voltage_mv. Battery == 0 means no
        // reading yet; treat as unrestricted.
        uint16_t v_batt = Lpf2::Battery::getCurrentVoltage();
        int32_t over_v_cap_pct =
            (v_batt > 0)
                ? std::min<int32_t>(100,
                                    (int32_t)s.max_voltage_mv * 100 / v_batt)
                : 100;
        int32_t eff_max = std::min(maxPwr, over_v_cap_pct);
        if (power_pct >  eff_max) power_pct =  eff_max;
        if (power_pct < -eff_max) power_pct = -eff_max;

        m_lastVoltageMv = (v_batt > 0) ? (power_pct * v_batt / 100)
                                       : (power_pct * s.max_voltage_mv / 100);
        applyPower((int8_t)power_pct);
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
        m_pidIntegral = 0.0;
        m_pidSpeedSetpointMdegps = 0;
        m_pidPrevPosErrValid = false;
        m_pidPosFineActive = false;
        if (style == BrakingStyle::HOLD)
        {
            m_pidMode = PidMode::HOLD;
            m_pidTarget = m_obsAngleMdeg;
            m_pidPositionFinal = m_obsAngleMdeg;
            m_pidPositionRampMdegps = 0;
            m_pidMaxPower = 100;
            m_pidEndTime = 0;
            m_pidLastMs = LPF2_GET_TIME();
        }
        else
        {
            m_pidMode = PidMode::NONE;
            m_lastVoltageMv = 0;
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
        if (speed == 0)
        {
            startPower(0);
            return;
        }
        if (m_pidMode == PidMode::SPEED && m_pidSpeed == speed && m_pidMaxPower == maxPower && m_pidEndTime == 0)
            return;
        m_pidSpeed = speed;
        m_pidMaxPower = maxPower;
        m_pidTarget = m_obsAngleMdeg;
        m_pidEndTime = 0;
        m_pidIntegral = 0.0;
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
        m_pidTarget = m_obsAngleMdeg;
        m_pidEndTime = newEndTime;
        m_pidEndState = endState;
        m_pidIntegral = 0.0;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::SPEED;
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_settings)
            return;
        int64_t signedDegMdeg = (int64_t)degrees * 1000 * (speed >= 0 ? 1 : -1);
        int64_t newFinal = m_obsAngleMdeg + signedDegMdeg;
        int32_t absSpeed = std::abs((int)speed);
        int32_t ramp =
            (int32_t)absSpeed * m_settings->rated_max_speed * 1000 / 100;
        if (m_pidMode == PidMode::POSITION &&
            m_pidPositionFinal == newFinal &&
            m_pidPositionRampMdegps == ramp &&
            m_pidMaxPower == maxPower &&
            m_pidEndState == endState &&
            m_pidEndTime == 0)
            return;
        m_pidPositionFinal = newFinal;
        m_pidPositionRampMdegps = ramp;
        m_pidTarget = m_obsAngleMdeg;
        m_pidMaxPower = maxPower;
        m_pidEndState = endState;
        m_pidEndTime = 0;
        m_pidIntegral = 0.0;
        m_pidPrevPosErrValid = false;
        m_pidPosFineActive = false;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::POSITION;
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_settings)
            return;
        int64_t newFinal = (int64_t)absPos * 1000;

        uint8_t newMaxPower = std::min(speed, maxPower);
        int32_t ramp =
            (int32_t)speed * m_settings->rated_max_speed * 1000 / 100;
        if (m_pidMode == PidMode::POSITION &&
            m_pidPositionFinal == newFinal &&
            m_pidPositionRampMdegps == ramp &&
            m_pidMaxPower == newMaxPower &&
            m_pidEndState == endState &&
            m_pidEndTime == 0)
            return;
        m_pidPositionFinal = newFinal;
        m_pidPositionRampMdegps = ramp;
        m_pidTarget = m_obsAngleMdeg;
        m_pidMaxPower = newMaxPower;
        m_pidEndState = endState;
        m_pidEndTime = 0;
        m_pidIntegral = 0.0;
        m_pidPrevPosErrValid = false;
        m_pidPosFineActive = false;
        m_pidLastMs = LPF2_GET_TIME();
        m_pidMode = PidMode::POSITION;
    }

    void Port::presetEncoder(int32_t pos)
    {
        m_currentRelPos = (int64_t)pos;
        m_obsAngleMdeg = (int64_t)pos * 1000;
        m_obsSpeedMdegps = 0;
        m_obsPrevMeasMdeg = m_obsAngleMdeg;
        m_obsPrevValid = false;
        m_obsInit = true;
        m_pidTarget = m_obsAngleMdeg;
        m_pidPositionFinal = m_obsAngleMdeg;
        m_pidPositionRampMdegps = 0;
        m_pidMode = PidMode::NONE;
        m_pidPrevPosErrValid = false;
        m_pidPosFineActive = false;
        m_lastVoltageMv = 0;
        setPower(0, 0);
    }
}; // namespace Lpf2::Local
