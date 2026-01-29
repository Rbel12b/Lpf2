#include "Lpf2Port.h"

void Lpf2Port::setMode(uint8_t num)
{
    if (num >= modes)
    {
        LPF2_LOG_W("Tried to set invalid mode %i (max %i)", num, modes - 1);
        return;
    }

    m_mode = num;
    uint8_t header = MESSAGE_CMD | CMD_SELECT;
    uint8_t checksum = header ^ 0xFF;
    checksum ^= (uint8_t)num;

    {
        MutexLock lock(m_serialMutex);
        m_serial->write(header);
        m_serial->write((uint8_t)num);
        m_serial->write(checksum);
        m_serial->flush();
    }

    if (modeData[num].flags.pin1())
    {
        m_pwm->out(255, 0);
    }
    if (modeData[num].flags.pin2())
    {
        m_pwm->out(0, 255);
    }

    LPF2_LOG_D("Set mode to %i (%s)", num, modeData[num].name.c_str());
}

void Lpf2Port::requestSpeedChange(uint32_t speed)
{
    uint8_t header = MESSAGE_CMD | CMD_SPEED | (2 << 3);
    uint8_t checksum = header ^ 0xFF;
    uint8_t b;
    {
        MutexLock lock(m_serialMutex);
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
    m_start = millis();
}

void Lpf2Port::changeBaud(uint32_t baud)
{
    MutexLock lock(m_serialMutex);
    m_serial->flush();
    m_serial->setBaudrate(baud);
}

void Lpf2Port::sendACK(bool NACK)
{
    MutexLock lock(m_serialMutex);
    m_serial->write(NACK ? BYTE_NACK : BYTE_ACK);
    m_serial->flush();
}

#define CHECK_LENGHT(size, msg_size, length) \
    else if (size <= length)                 \
    {                                        \
        msg_size = LENGTH_##length;          \
    }

int Lpf2Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
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
        MutexLock lock(m_serialMutex);
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

void Lpf2Port::setPower(uint8_t pin1, uint8_t pin2)
{
    if ((m_dumb || (modeData.size() > m_mode && modeData[m_mode].flags.power12())) && m_pwm)
    {
        m_pwm->out(pin1, pin2);
    }
}