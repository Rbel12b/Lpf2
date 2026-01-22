#include "EncoderMotor.h"

const CapabilityId EncoderMotor::CAP =
    CapabilityRegistry::instance().registerCapability("encoder_motor");
const uint8_t EncoderMotor::CALIB_MODE = 4;

namespace
{
    EncoderMotorFactory factory;

    const bool registered = []
    {
        Lpf2DeviceRegistry::instance().registerFactory(&factory);
        return true;
    }();
}

bool EncoderMotor::hasCapability(CapabilityId id) const
{
    return id == CAP;
}

void *EncoderMotor::getCapability(CapabilityId id)
{
    if (id == CAP)
        return static_cast<EncoderMotorControl *>(this);
    return nullptr;
}

bool EncoderMotorFactory::matches(Lpf2Port &port) const
{
    switch (port.getDeviceType())
    {
    case Lpf2DeviceType::MEDIUM_LINEAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_LARGE_LINEAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_XLARGE_LINEAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY:
    case Lpf2DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR_GREY:
        return true;
    default:
        break;
    }
    return false;
}

void EncoderMotor::resetPid()
{
    m_pidIntegral = 0.0f;
    m_pidLastError = 0.0f;
}

int EncoderMotor::pidStep(float error)
{
    // Integral
    m_pidIntegral += error;
    if (m_pidIntegral > m_pidIntegralLimit)
        m_pidIntegral = m_pidIntegralLimit;
    else if (m_pidIntegral < -m_pidIntegralLimit)
        m_pidIntegral = -m_pidIntegralLimit;

    // Derivative (difference only)
    float derivative = error - m_pidLastError;
    m_pidLastError = error;

    // PID output
    float output =
        m_kp * error +
        m_ki * m_pidIntegral +
        m_kd * derivative;

    // Clamp to max speed
    if (output > m_moveSpeed)
        output = static_cast<float>(m_moveSpeed);
    else if (output < -static_cast<float>(m_moveSpeed))
        output = -static_cast<float>(m_moveSpeed);

    return static_cast<int>(output);
}


void EncoderMotor::poll()
{
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

    LPF2_LOG_D("AbsPos: %u, RelPos: %lld, Delta: %d",
               absPos, m_currentRelPos, delta);

    switch (m_mode)
    {
    case Mode::SPEED:
        // Manual control
        break;

    case Mode::MOVE_DEGREES:
    {
        if (m_deg == 0)
        {
            setSpeed(0);
            break;
        }

        m_deg -= delta;
        float error = static_cast<float>(m_deg);

        if ((error > 0 && m_deg <= 0) ||
            (error < 0 && m_deg >= 0))
        {
            m_deg = 0;
            m_holdRelPos = m_currentRelPos;
            resetPid();
            m_mode = Mode::HOLD;
            setSpeed(0);
        }
        else
        {
            setSpeed(pidStep(error));
        }
        break;
    }

    case Mode::MOVE_TO_ABS:
    {
        int16_t diff =
            static_cast<int16_t>(m_absPos) -
            static_cast<int16_t>(absPos);

        if (diff > 1800)
            diff -= 3600;
        else if (diff < -1800)
            diff += 3600;

        float error = static_cast<float>(diff);

        if (std::abs(error) <= 10.0f)
        {
            m_holdAbsPos = m_absPos;
            resetPid();
            m_mode = Mode::HOLD;
            setSpeed(0);
        }
        else
        {
            setSpeed(pidStep(error));
        }
        break;
    }

    case Mode::MOVE_TO_REL:
    {
        float error =
            static_cast<float>(
                static_cast<int64_t>(m_relPos) -
                static_cast<int64_t>(m_currentRelPos));

        if (std::abs(error) <= 10.0f)
        {
            m_holdRelPos = m_relPos;
            resetPid();
            m_mode = Mode::HOLD;
            setSpeed(0);
        }
        else
        {
            setSpeed(pidStep(error));
        }
        break;
    }

    case Mode::HOLD:
    {
        // Prefer relative hold if available
        float error;

        if (m_holdRelPos != 0)
        {
            error =
                static_cast<float>(
                    static_cast<int64_t>(m_holdRelPos) -
                    static_cast<int64_t>(m_currentRelPos));
        }
        else
        {
            int16_t diff =
                static_cast<int16_t>(m_holdAbsPos) -
                static_cast<int16_t>(absPos);

            if (diff > 1800)
                diff -= 3600;
            else if (diff < -1800)
                diff += 3600;

            error = static_cast<float>(diff);
        }

        if (std::abs(error) < 10.0f) // 1°
        {
            setSpeed(0);
            m_pidIntegral = 0.0f;
            break;
        }

        setSpeed(pidStep(error));
        break;
    }
    }
}

uint16_t EncoderMotor::getAbsPos() const
{
    return (uint16_t)(port_.getValue(CALIB_MODE, 0) / 1024.0f * 3600.0f);
}

uint64_t EncoderMotor::getRelPos() const
{
    return m_currentRelPos;
}

void EncoderMotor::setRelPos(uint64_t pos)
{
    m_currentRelPos = pos;
}

void EncoderMotor::setSpeed(int speed)
{
    m_mode = Mode::SPEED;
    bool forward = speed >= 0;
    speed = std::abs(speed);
    if (speed > 100)
        speed = 100;

    uint8_t pwr2 = speed * 0xFF / 100;
    uint8_t pwr1 = 0;
    if (!forward)
        std::swap(pwr1, pwr2);

    port_.setPower(pwr1, pwr2);
}

void EncoderMotor::moveToAbsPos(uint16_t pos, uint8_t speed)
{
    m_moveSpeed = speed;
    m_absPos = pos * 10;
    resetPid();
    m_mode = Mode::MOVE_TO_ABS;
}

void EncoderMotor::moveToRelPos(uint64_t pos, uint8_t speed)
{
    m_moveSpeed = speed;
    m_relPos = pos * 10;
    resetPid();
    m_mode = Mode::MOVE_TO_REL;
}

void EncoderMotor::moveDegrees(int64_t degrees, uint8_t speed)
{
    m_moveSpeed = speed;
    m_deg = degrees * 10;
    resetPid();
    m_mode = Mode::MOVE_DEGREES;
}
