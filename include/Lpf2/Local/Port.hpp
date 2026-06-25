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
#include "Lpf2/Port.hpp"
#include "Lpf2/Local/Serial.hpp"
#include "Lpf2/Local/SerialDef.hpp"
#include "Lpf2/Util/mutex.hpp"

#define MEASUREMENTS 20

namespace Lpf2::Local
{
    // All per-motor-type tuning constants. Definitions in PortPID.cpp.
    struct MotorSettings
    {
        DeviceType id;
        int32_t rated_max_speed;   // deg/s (informational)
        int32_t max_voltage_mv;    // motor's nominal max drive voltage

        // SPEED-mode pct-domain PID.
        float speed_ksp;           // power_pct per pct_err
        float speed_ksi;           // power_pct per (pct_err·s)
        float speed_int_clamp;     // pct·s
        float speed_deadband_pct;

        // POSITION/HOLD pct-domain PID (fine sub-mode).
        float pos_kp;              // power_pct per deg
        float pos_ki;              // power_pct per (deg·s)
        float pos_kd;              // power_pct per (deg/s) — true derivative on pos err
        float pos_int_clamp;
        float pos_deadband_deg;
        double pos_decel_mdps2;    // trapezoidal decel cap
        float pos_handoff_deg;     // |remaining| under which fine sub-mode engages

        // Friction comp (pct).
        float breakaway_pct;       // stiction kick
        float kinetic_floor_pct;   // sustained drive floor
    };

    // Per-motor tuning singletons. Mutable at runtime; PID loop reads on tick.
    extern MotorSettings MS_MEDIUM_LINEAR_MOTOR;
    extern MotorSettings MS_TECHNIC_LARGE_LINEAR_MOTOR;
    extern MotorSettings MS_TECHNIC_XLARGE_LINEAR_MOTOR;
    extern MotorSettings MS_TECHNIC_MEDIUM_ANGULAR_MOTOR;
    extern MotorSettings MS_TECHNIC_LARGE_ANGULAR_MOTOR;
    extern MotorSettings MS_TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY;
    extern MotorSettings MS_TECHNIC_LARGE_ANGULAR_MOTOR_GREY;

    class Port : public Lpf2::Port
    {
    public:
        Port() = delete;
        Port(IO &IO) : m_IO(IO), m_serial(m_IO.getUart()), m_pwm(m_IO.getPWM()) {};

        void init(
#if defined(LPF2_USE_FREERTOS)
            bool useFreeRTOS = true,
            std::string taskName = "uartTask"
#endif
        );

        void _update() override;

        enum class STATUS
        {
            /* Something bad happened. */
            STATUS_ERR,
            /* Waiting for data that looks like LEGO UART protocol. */
            STATUS_SYNCING,
            /* Reading device info before changing baud rate. */
            STATUS_INFO,
            /* Waiting for ACK */
            STATUS_ACK_WAIT,
            /* Waiting for SYNC */
            STATUS_SYNC_WAIT,
            /* Sending ACK, for data phase begin */
            STATUS_ACK_SENDING,
            /* Sending speed change request */
            STATUS_SPEED_CHANGE,
            /* Speed change accepted */
            STATUS_SPEED,
            /* Waiting for first data packet */
            STATUS_DATA_RECEIVED,
            /* First data packet received, sending setup data */
            STATUS_DATA_START,
            /* Normal data receiving state */
            STATUS_DATA,
            /* Analog identification */
            STATUS_ANALOD_ID,
        };

        int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
        void setPower(uint8_t pin1, uint8_t pin2);
        bool isDeviceConnected() override;

        int setMode(uint8_t mode, float delta = 1.0f) override;
        int setModeCombo(uint8_t idx, const std::vector<float>& deltas = {}) override;

        void startPower(int8_t pw) override;
        void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) override;
        void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) override;
        void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile = 0) override;
        void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void presetEncoder(int32_t pos) override;

        void updateMotorPID();

    private:
#if defined(LPF2_USE_FREERTOS)
        static void taskEntryPoint(void *pvParameters);
        void uartTask();
#endif

        void parseMessage(const Message &msg);
        void parseMessageCMD(const Message &msg);
        void parseMessageInfo(const Message &msg);
        void changeBaud(uint32_t baud);
        void sendACK(bool NACK = false);
        void requestSpeedChange(uint32_t speed);

        void resetDevice();
        void enterUartState();

        uint8_t process(unsigned long now);

        void doAnalogID();

        size_t getSpeed() const { return m_baud; }

    private:
        STATUS m_status = STATUS::STATUS_ERR;
        STATUS m_new_status = STATUS::STATUS_ERR;
        STATUS m_lastStatus = STATUS::STATUS_ERR;
        uint32_t m_baud = 2400;
        bool m_deviceConnected = false; // do not rely on this, use isDeviceConnected() instead
        bool nextModeExt = false;
        bool m_dumb = false;
        IO &m_IO;
        Uart *m_serial;
        PWM *m_pwm;
        Parser m_parser;
        Writer m_writer;

#ifdef LPF2_MUTEX_INVALID
        Lpf2::Utils::Mutex m_serialMutex = LPF2_MUTEX_INVALID;
#else
        Lpf2::Utils::Mutex m_serialMutex;
#endif

        /**
         * Time of the last data received (millis since startup).
         */
        uint64_t m_startRec = 0;

        /**
         * Time of the start of the current operation (millis since startup).
         */
        uint64_t m_start = 0;

        uint8_t m_mode = 0;
        int8_t m_activeCombo = -1; // -1 = single mode, >=0 = active combo index

        float ch0Measurements[MEASUREMENTS];
        float ch1Measurements[MEASUREMENTS];
        uint8_t measurementNum = 0;
        uint64_t lastMeasurement = 0;

        const int m_detectionThreshold = 5; // Number of consecutive detections required - 1, so 2 means 3 times
        int m_detectionCounter = 0;
        int m_lastDetectedType = -1;

        bool m_deviceDataReceived = false;

    private:
        int32_t getMotorPos() const
        {
            if (m_activeCombo >= 0)
                return (int32_t)getValue(2, 0); // POS mode (mode 2), whole degrees, accumulates
            return (int32_t)(getValue((uint8_t)ModeNum::MOTOR__CALIB, 0) / 1024.0f * 360.0f);
        }

        int64_t m_currentRelPos = 0;
        int32_t m_lastMotorPos = 0;

        enum class PidMode : uint8_t { NONE, SPEED, POSITION, HOLD };

        PidMode m_pidMode = PidMode::NONE;
        // m_pidTarget in mdeg
        int64_t m_pidTarget = 0;
        int8_t m_pidSpeed = 0;
        uint8_t m_pidMaxPower = 100;
        BrakingStyle m_pidEndState = BrakingStyle::FLOAT;
        uint64_t m_pidEndTime = 0;

        // Per-motor-type settings (resolved on device attach).
        const MotorSettings *m_settings = nullptr;

        // Observer state.
        int64_t m_obsAngleMdeg = 0;
        int32_t m_obsSpeedMdegps = 0;
        int32_t m_lastVoltageMv = 0;
        bool m_obsInit = false;

        // Speed setpoint (mdeg/s) re-derived each PID tick from m_pidSpeed.
        int32_t m_pidSpeedSetpointMdegps = 0;

        // Position-mode trajectory: target ramps toward final at ramp speed.
        int64_t m_pidPositionFinal = 0;     // mdeg
        int32_t m_pidPositionRampMdegps = 0; // |ramp speed|

        // Previous encoder reading (mdeg) for speed derivation.
        int64_t m_obsPrevMeasMdeg = 0;
        bool m_obsPrevValid = false;

        // Integral accumulator (units depend on mode/sub-mode).
        double m_pidIntegral = 0.0;
        uint64_t m_pidLastMs = 0;

        // POSITION fine sub-mode state.
        float m_pidPrevPosErrDeg = 0.0f;
        bool m_pidPrevPosErrValid = false;
        bool m_pidPosFineActive = false;

        void applyPower(int8_t pw);
        void applyEndState(BrakingStyle style);
    };
}; // namespace Lpf2::Local