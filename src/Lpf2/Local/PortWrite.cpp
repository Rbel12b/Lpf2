#include "Lpf2/Local/Port.hpp"

namespace Lpf2::Local
{
    int Port::setMode(uint8_t mode)
    {
        if (mode >= modes)
        {
            LPF2_LOG_W("Tried to set invalid mode %i (max %i)", mode, modes - 1);
            return 1;
        }

        m_mode = mode;
        uint8_t header = MESSAGE_CMD | CMD_SELECT;
        uint8_t checksum = header ^ 0xFF;
        checksum ^= (uint8_t)mode;

        {
            Utils::MutexLock lock(m_serialMutex);
            m_serial->write(header);
            m_serial->write((uint8_t)mode);
            m_serial->write(checksum);
            m_serial->flush();
        }

        if (modeData[mode].flags.pin1())
        {
            LPF2_LOG_D("Setting pin1 high, pin2 low");
            m_pwm->out(255, 0);
        }
        else if (modeData[mode].flags.pin2())
        {
            LPF2_LOG_D("Setting pin2 high, pin1 low");
            m_pwm->out(0, 255);
        }

        LPF2_LOG_D("Set mode to %i (%s)", mode, modeData[mode].name.c_str());
        return 0;
    }

    int Port::setModeCombo(uint8_t idx)
    {
        LPF2_LOG_W("Set mode Combo: %i, unimplemented!", idx);
        return 0;
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
        // TODO: implement this
    }

    void Port::requestSpeedChange(uint32_t speed)
    {
        uint8_t header = MESSAGE_CMD | CMD_SPEED | (2 << 3);
        uint8_t checksum = header ^ 0xFF;
        uint8_t b;
        {
            Utils::MutexLock lock(m_serialMutex);
            m_serial->write(header);
            b = (speed & 0xFF) >> 0;
            checksum ^= b;
            m_serial->write(b);
            b = (speed & 0xFF00) >> 8;
            checksum ^= b;
            m_serial->write(b);
            b = (speed & 0xFF0000) >> 16;
            checksum ^= b;
            m_serial->write(b);
            b = (speed & 0xFF000000) >> 24;
            checksum ^= b;
            m_serial->write(b);
            m_serial->write(checksum);
            m_serial->flush();
        }
        m_status = LPF2_STATUS::STATUS_ACK_WAIT;
        m_new_status = LPF2_STATUS::STATUS_SPEED;
        m_start = LPF2_GET_TIME();
    }

    void Port::changeBaud(uint32_t baud)
    {
        Utils::MutexLock lock(m_serialMutex);
        m_serial->flush();
        m_serial->setBaudrate(baud);
    }

    void Port::sendACK(bool NACK)
    {
        Utils::MutexLock lock(m_serialMutex);
        m_serial->write(NACK ? BYTE_NACK : BYTE_ACK);
        m_serial->flush();
    }

#define CHECK_LENGHT(size, msg_size, length) \
    else if (size <= length)                 \
    {                                        \
        msg_size = LENGTH_##length;          \
    }

    int Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
    {
        if (!deviceConnected())
        {
            return 1;
        }
        if (modeNum >= modeData.size())
        {
            return 1;
        }

        size_t size = data.size();
        uint8_t msg_size = 0;

        if (size == 0)
        {
            return 0;
        }
        CHECK_LENGHT(size, msg_size, 1)
        CHECK_LENGHT(size, msg_size, 2)
        CHECK_LENGHT(size, msg_size, 4)
        CHECK_LENGHT(size, msg_size, 8)
        CHECK_LENGHT(size, msg_size, 16)
        CHECK_LENGHT(size, msg_size, 32)
        CHECK_LENGHT(size, msg_size, 64)
        CHECK_LENGHT(size, msg_size, 128)
        else if (size > 128)
        {
            return 1;
        }

        uint8_t header = MESSAGE_CMD | CMD_EXT_MODE | LENGTH_1;
        uint8_t checksum = header ^ 0xFF;
        uint8_t b = (modeNum >= 8) ? 8 : 0;

        {
            Utils::MutexLock lock(m_serialMutex);
            m_serial->write(header);
            checksum ^= b;
            m_serial->write(b);
            m_serial->write(checksum);

            header = MESSAGE_DATA | msg_size | (modeNum & 0x07);
            checksum = header ^ 0xFF;
            m_serial->write(header);

            for (uint8_t i = 0; i < size; i++)
            {
                b = data[i];
                checksum ^= b;
                m_serial->write(b);
            }

            m_serial->write(checksum);
            m_serial->flush();
        }

        return 0;
    }

    void Port::setPower(uint8_t pin1, uint8_t pin2)
    {
        if ((m_dumb || (modeData.size() > m_mode && modeData[m_mode].flags.power12())) && m_pwm)
        {
            m_pwm->out(pin1, pin2);
        }
    }
}; // namespace Lpf2::Local