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

namespace Lpf2::Local
{
    int Port::setMode(uint8_t mode, float delta)
    {
        if (mode >= m_modeCount)
        {
            LPF2_LOG_W("Tried to set invalid mode %i (max %i)", mode, m_modeCount - 1);
            return 1;
        }

        storeModeDelta(mode, delta);
        m_activeCombo = -1;
        m_mode = mode;

        uint8_t modeInBank = mode & 0x07;
        uint8_t selectHeader = MESSAGE_CMD | LENGTH_1 | CMD_SELECT;

        {
            Utils::MutexLock lock(m_serialMutex);
            if (mode >= 8)
            {
                uint8_t extHeader = MESSAGE_CMD | LENGTH_1 | CMD_EXT_MODE;
                m_serial->write(extHeader);
                m_serial->write((uint8_t)0x08);
                m_serial->write((uint8_t)(extHeader ^ 0xFF ^ 0x08));
            }
            m_serial->write(selectHeader);
            m_serial->write(modeInBank);
            m_serial->write((uint8_t)(selectHeader ^ 0xFF ^ modeInBank));
            m_serial->flush();
        }

        if (mode < m_modeData.size())
        {
            if (m_modeData[mode].flags.pin1())
            {
                LPF2_LOG_D("Setting pin1 high, pin2 low");
                m_pwm->out(255, 0);
            }
            else if (m_modeData[mode].flags.pin2())
            {
                LPF2_LOG_D("Setting pin2 high, pin1 low");
                m_pwm->out(0, 255);
            }
        }

        LPF2_LOG_D("Set mode to %i", mode);
        return 0;
    }

    int Port::setModeCombo(uint8_t idx, const std::vector<float>& deltas)
    {
        if (idx >= m_comboNum || m_modeCombos[idx] == 0)
        {
            LPF2_LOG_W("Invalid combo index: %i (max %i)", idx, (int)m_comboNum - 1);
            return 1;
        }
        storeComboDeltas(idx, deltas);

        uint16_t bitmask = m_modeCombos[idx];
        // CMD_WRITE activates combined mode. Format confirmed from LEGO Technic hub captures.
        // Byte 0: 0x20 | num_pairs (bit 5 = combined mode flag, bits 0-3 = pair count).
        // Byte 1: combo index.
        // Bytes 2+: (mode<<4)|dataset nibble pairs in ascending mode order.
        // Writer pads to next power-of-2 automatically.
        uint8_t numPairs = (uint8_t)__builtin_popcount(bitmask);
        Message msg;
        msg.msg = MESSAGE_CMD;
        msg.cmd = CMD_WRITE;
        msg.data.push_back(0x20 | numPairs);
        msg.data.push_back(idx);
        for (int m = 0; m < 16; m++)
        {
            if (bitmask & (1u << m))
                msg.data.push_back((uint8_t)((m << 4) | 0x00));
        }

        {
            Utils::MutexLock lock(m_serialMutex);
            m_writer.write(msg);
        }

        m_activeCombo = (int8_t)idx;
        LPF2_LOG_D("Set combo %i (bitmask 0x%04X)", idx, bitmask);

        // set m_mode to the first mode in the combo for internal state tracking
        for (int m = 0; m < 16; m++)
        {
            if (bitmask & (1u << m))
            {
                m_mode = (uint8_t)m;
                break;
            }
        }
        return 0;
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
        m_status = STATUS::STATUS_ACK_WAIT;
        m_new_status = STATUS::STATUS_SPEED;
        m_start = LPF2_GET_TIME();
    }

    void Port::changeBaud(uint32_t m_baud)
    {
        Utils::MutexLock lock(m_serialMutex);
        m_serial->flush();
        m_serial->setBaudrate(m_baud);
    }

    void Port::sendACK(bool NACK)
    {
        Utils::MutexLock lock(m_serialMutex);
        LPF2_LOG_V("Sending %s", NACK ? "NACK" : "ACK");
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
        if (!isDeviceConnected())
        {
            return 1;
        }
        LPF2_LOG_D("writeData: mode %i, data size %i", modeNum, (int)data.size());
        if (deviceIsMotor(m_deviceType) && modeNum == 0 && data.size())
        {
            LPF2_LOG_D("startPower: %i", (int8_t)data[0]);
            int8_t speed = data[0];
            if (speed == 127)
                speed = 0;
            startPower(speed);
            return 0;
        }

        if (modeNum >= m_modeData.size())
        {
            return 1;
        }

        {
            Utils::MutexLock lock(m_serialMutex);
            if (modeNum >= 8)
            {
                Message msg;
                msg.msg = MESSAGE_CMD;
                msg.cmd = CMD_EXT_MODE;
                msg.data.push_back(8);
                m_writer.write(msg);
            }

            Message msg;
            msg.msg = MESSAGE_DATA;
            msg.cmd = modeNum & 0x07;
            msg.data = data;
            m_writer.write(msg);
        }

        return 0;
    }

    void Port::setPower(uint8_t pin1, uint8_t pin2)
    {
        if ((m_dumb || deviceIsMotor(m_deviceType) || (m_modeData.size() > m_mode && m_modeData[m_mode].flags.power12())) && m_pwm)
        {
            m_pwm->out(pin1, pin2);
        }
    }
}; // namespace Lpf2::Local