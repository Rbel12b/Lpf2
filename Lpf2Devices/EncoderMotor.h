#pragma once
#ifndef _LPF2_ENCODER_MOTOR_H_
#define _LPF2_ENCODER_MOTOR_H_

#include "../config.h"
#include "../Lpf2Device.h"

class EncoderMotorControl
{
public:
    virtual ~EncoderMotorControl() = default;

    /**
     * @brief Get the absolute position of the motor.
     * @return Position in 1/10 of degrees (0-360 degrees = 0-3600)
     */
    virtual uint16_t getAbsPos() const = 0;

    /**
     * @brief Get the relative position of the motor.
     * @return Position in degrees from the zero point (can be negative).
     */
    virtual uint64_t getRelPos() const = 0;

    /**
     * @brief Set the relative position of the motor (set the zero point).
     * @param pos Position in degrees from the zero point.
     */
    virtual void setRelPos(uint64_t pos) = 0;

    /**
     * @brief Set the speed of the motor.
     * @param speed Speed in percentage (-100 to 100).
     */
    virtual void setSpeed(int speed) = 0;

    /**
     * @brief Move the motor to an absolute position.
     * @param pos Position in degrees.
     * @param speed Speed in percentage (1 to 100).
     */
    virtual void moveToAbsPos(uint16_t pos, uint8_t speed) = 0;

    /**
     * @brief Move the motor to a relative position.
     * @param pos Position in degrees from the zero point.
     * @param speed Speed in percentage (1 to 100).
     */
    virtual void moveToRelPos(uint64_t pos, uint8_t speed) = 0;

    /**
     * @brief Move the motor by a number of degrees.
     * @param degrees Degrees to move (can be negative).
     * @param speed Speed in percentage (1 to 100).
     */
    virtual void moveDegrees(int64_t degrees, uint8_t speed) = 0;
};

class EncoderMotor : public Lpf2Device, public EncoderMotorControl
{
public:
    EncoderMotor(Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override
    {
        setSpeed(0);
        return true;
    }

    void poll() override;

    const char *name() const override
    {
        return "Motor with Encoder";
    }

    uint16_t getAbsPos() const override;
    uint64_t getRelPos() const override;
    void setRelPos(uint64_t pos) override;
    void setSpeed(int speed) override;
    void moveToAbsPos(uint16_t pos, uint8_t speed) override;
    void moveToRelPos(uint64_t pos, uint8_t speed) override;
    void moveDegrees(int64_t degrees, uint8_t speed) override;

    bool hasCapability(CapabilityId id) const override;
    void *getCapability(CapabilityId id) override;

    const static CapabilityId CAP;
    const static uint8_t CALIB_MODE;

private:
    void resetPid();
    int pidStep(float error);

    enum class Mode
    {
        SPEED,
        MOVE_DEGREES,
        MOVE_TO_ABS,
        MOVE_TO_REL,
        HOLD
    };
    Mode m_mode;
    union
    {
        uint16_t m_absPos = 0;
        uint64_t m_relPos;
        uint16_t m_deg;
    };
    uint8_t m_moveSpeed = 0;
    uint64_t m_currentRelPos = 0;
    uint16_t m_lastAbsPos = 0;

    // PID state
    float m_pidIntegral = 0.0f;
    float m_pidLastError = 0.0f;

    // PID gains
    float m_kp = 0.08f;
    float m_ki = 0.0f;
    float m_kd = 0.006f;

    // Anti-windup
    float m_pidIntegralLimit = 2000.0f;

    // Hold targets
    uint16_t m_holdAbsPos = 0;
    uint64_t m_holdRelPos = 0;
};

class EncoderMotorFactory : public Lpf2DeviceFactory
{
public:
    bool matches(Lpf2Port &port) const override;

    Lpf2Device *create(Lpf2Port &port) const override
    {
        return new EncoderMotor(port);
    }

    const char *name() const
    {
        return "Technic Color Sensor Factory";
    }
};

#endif