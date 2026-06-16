#include "Lpf2/Local/EmulatedPort.hpp"
#include <cstring>

namespace Lpf2::Local
{
    void EmulatedPort::attachDevice(Lpf2::Virtual::Device &device)
    {
        if (m_device)
        {
            LPF2_LOG_W("Device already attached to port, detaching it first");
            detachDevice();
        }
        m_device = &device;
        m_device->setValueChangeCallback(std::bind(&EmulatedPort::deviceValueChangeCallback, this, std::placeholders::_1));
    }

    void EmulatedPort::detachDevice()
    {
        m_device = nullptr;
        m_comboPairs.clear();
    }

    void EmulatedPort::init()
    {
        if (!m_serial)
        {
            return;
        }
        m_baud = 115200;
        m_serial->uartPinsOn();
        changeBaud(m_baud);
        m_writer.init(m_serial);
        m_parser.init(m_serial);
        reset();
    }

    void EmulatedPort::deviceValueChangeCallback(uint8_t modeNum)
    {
        sendUpdate(modeNum);
    }

    void EmulatedPort::update()
    {
        if (!m_serial || !m_device)
        {
            return;
        }

        m_device->update();
        
        auto messages = m_parser.update(100);

        for (const auto &msg : messages)
        {
            LPF2_DEBUG_EXPR_V(
                m_parser.printMessage(msg);
            );
            parseMessage(msg);
        }

        switch (m_status)
        {
        case STATUS::DETECTING_HOST:
            LPF2_LOG_D("Asserting TX=low");
            m_status = STATUS::WAITING_FOR_HOST;
            m_serial->uartPinsOn();
            m_baud = 115200;
            m_serial->setBaudrate(m_baud);
            m_serial->writeCh(1, false);
            LPF2_LOG_D("Asserted TX=low");
            m_start = LPF2_GET_TIME();
            break;
        case STATUS::WAITING_FOR_HOST:
            // m_serial->write(0xAA);
            if (LPF2_GET_TIME() - m_start >= 1000)
            {
                m_hostType = HostType::EV3;
                m_status = STATUS::HOST_DETECTED;
                m_baud = 2400;
                m_serial->uartPinsOn();
                m_serial->setBaudrate(m_baud);
                LPF2_LOG_D("LPF2 host timed out, failing back to EV3");
            }
            break;
    
        case STATUS::HOST_DETECTED:
            m_status = STATUS::SENDING_INFO;
            break;

        case STATUS::SENDING_INFO:
            handleSendingInfo();
            break;

        case STATUS::SENDING_DATA:
            // if (m_device->getModes().size() > m_mode && m_lastModeData == m_device->getModes()[m_mode].rawData)
            // {
            //     sendUpdate();
            // }
            // [[fallthrough]];
        case STATUS::WAITING_FOR_ACK:
            if ((LPF2_GET_TIME() - m_start) > 1000)
            {
                reset();
            }
            break;
        
        default:
            break;
        }
    }

    bool EmulatedPort::isHostConnected()
    {
        return m_status == STATUS::SENDING_DATA;
    }

    void EmulatedPort::parseMessage(const Message &msg)
    {
        if (msg.header != BYTE_SYNC)
        {
            m_start = LPF2_GET_TIME();
        }
        if (msg.header == BYTE_ACK && (m_status == STATUS::WAITING_FOR_ACK || m_status == STATUS::SENDING_INFO))
        {
            changeBaud(m_baud);
            m_status = STATUS::SENDING_DATA;
        }
        else if (msg.header == BYTE_NACK && m_status == STATUS::SENDING_DATA)
        {
            sendUpdate();
        }
        else if (msg.msg == MESSAGE_CMD && msg.cmd == CMD_SPEED) // && m_status == STATUS::WAITING_FOR_HOST)
        {
            if (msg.length != 4)
            {
                return;
            }
            m_baud = msg.data[0] | ((uint64_t)msg.data[1] << 8) | ((uint64_t)msg.data[2] << 16) | ((uint64_t)msg.data[3] << 24);
            m_serial->flush();
            m_serial->uartPinsOn();
            changeBaud(m_baud);
            sendACK();
            m_serial->flush();
            m_hostType = HostType::LPF2;
            m_status = STATUS::HOST_DETECTED;
            LPF2_LOG_D("Detected LPF2 host, with speed: %i", m_baud);
        }
        else if (msg.msg == MESSAGE_CMD && msg.cmd == CMD_SELECT)
        {
            if (msg.length != 1)
            {
                return;
            }
            m_comboPairs.clear();
            m_mode = msg.data[0];
            LPF2_LOG_D("Selected mode: %i, msg: %s", m_mode, Utils::bytes_to_hexString(msg.data).c_str());
        }
        else if (msg.msg == MESSAGE_CMD && msg.cmd == CMD_EXT_MODE)
        {
            if (msg.data.size() >= 1)
                m_nextModeExt = msg.data[0];
        }
        else if (msg.msg == MESSAGE_CMD && msg.cmd == CMD_WRITE)
        {
            // byte 0: 0x20 | numPairs (bit 5 = combined mode flag, low nibble = pair count)
            // byte 1: combo index
            // bytes 2+: (mode<<4)|dataset pairs
            if (msg.data.size() < 2)
                return;
            uint8_t subCmd = msg.data[0];
            if ((subCmd & 0xF0) != 0x20)
            {
                LPF2_LOG_W("CMD_WRITE: unsupported subcommand 0x%02X", (int)subCmd);
                return;
            }
            uint8_t numPairs = subCmd & 0x0F;
            uint8_t comboIdx = msg.data[1];

            Message echo = msg;
            m_writer.write(echo);

            if (numPairs == 0)
            {
                m_comboPairs.clear();
                LPF2_LOG_D("CMD_WRITE: combined mode reset");
                return;
            }

            auto combos = m_device->getModeCombos();
            if (comboIdx >= combos.size() || combos[comboIdx] == 0)
            {
                LPF2_LOG_W("CMD_WRITE: invalid combo index %i", (int)comboIdx);
                m_comboPairs.clear();
                return;
            }
            m_comboPairs.clear();
            for (uint8_t i = 0; i < numPairs && (2 + i) < msg.data.size(); i++)
                m_comboPairs.push_back(msg.data[2 + i]);
            LPF2_LOG_D("CMD_WRITE: combo %i, %d pairs", (int)comboIdx, (int)m_comboPairs.size());
        }
        else if (msg.msg == MESSAGE_DATA)
        {
            uint8_t mode = msg.cmd + m_nextModeExt;
            m_nextModeExt = 0;
            m_device->writeData(mode, msg.data);
        }
        else if (msg.msg != MESSAGE_SYS)
        {
            LPF2_LOG_W("Unknown message: ↓, status: %i", (int)m_status);
            m_parser.printMessage(msg);
        }
    }

    void EmulatedPort::sendUpdate(uint8_t modeNum)
    {
        if (!m_comboPairs.empty())
        {
            // Combined mode: concatenate rawData for each (mode<<4)|dataset pair.
            // Header uses cmd=0 (mode bank 0), confirmed from captures.
            Message msg;
            msg.msg = MESSAGE_DATA;
            msg.cmd = 0;
            for (uint8_t pair : m_comboPairs)
            {
                uint8_t pairMode = (pair >> 4) & 0x0F;
                if (m_device->getModes().size() > pairMode)
                {
                    const auto &raw = m_device->getModes()[pairMode].rawData;
                    msg.data.insert(msg.data.end(), raw.begin(), raw.end());
                }
            }
            m_writer.write(msg);
            return;
        }

        if (modeNum == 0xFF)
        {
            modeNum = m_mode;
        }

        if (modeNum >= 8)
        {
            Message msg;
            msg.msg = MESSAGE_CMD;
            msg.cmd = CMD_EXT_MODE;
            msg.data.push_back(8);
            m_writer.write(msg);
        }
        Mode mode;
        if (m_device->getModes().size() > modeNum)
        {
            mode = m_device->getModes()[modeNum];
        }
        Message msg;
        msg.msg = MESSAGE_DATA;
        msg.cmd = modeNum & 0x07;
        msg.data.insert(msg.data.end(), mode.rawData.begin(), mode.rawData.end());
        m_writer.write(msg);
    }

    void EmulatedPort::handleSendingInfo()
    {
        LPF2_LOG_V("Sending info: state: %i, %i, %i", (int)m_infoState, (int)m_infoNum, (int)m_infoSubNum);
        Message msg;
        bool okayToWrite = true;
        switch (m_infoState)
        {
        case InfoState::CMD:
            msg.msg = MESSAGE_CMD;
            switch (m_infoNum)
            {
            case 0:
                m_serial->write(BYTE_SYNC);
                msg.cmd = CMD_TYPE;
                msg.data.push_back((uint8_t)m_device->getDeviceType());
                break;

            case 1:
                msg.cmd = CMD_MODES;
                msg.data.push_back((m_device->getModeCount() - 1) & 0x07);
                msg.data.push_back((m_device->getModeCount() - 1) & 0x07); // views
                msg.data.push_back((m_device->getModeCount() - 1));
                msg.data.push_back((m_device->getModeCount() - 1)); // views
                break;

            case 2:
                msg.cmd = CMD_SPEED;
                msg.data.resize(4);
                m_baud = 115200;
                std::memcpy(msg.data.data(), &m_baud, 4);
                break;

            case 3:
            {
                msg.cmd = CMD_VERSION;
                auto fw = Utils::packVersion(m_device->getFwVersion());
                msg.data.insert(msg.data.end(), fw.begin(), fw.end());
                auto hw = Utils::packVersion(m_device->getHwVersion());
                msg.data.insert(msg.data.end(), hw.begin(), hw.end());
                break;
            }

            default:
                okayToWrite = false;
                m_infoState = InfoState::MODE;
                m_infoNum = 0xFF; // overflows to 0
                m_infoSubNum = 0;
                if (m_device->getModeCount() == 0)
                {
                    sendACK();
                    m_status = STATUS::WAITING_FOR_ACK;
                    m_start = LPF2_GET_TIME();
                }
                break;
            }
            m_infoNum++;
            break;

        case InfoState::MODE:
        {
            msg.data.reserve(16);
            msg.msg = MESSAGE_INFO;
            msg.cmd = (m_infoNum >= 8 ? m_infoNum - 8 : m_infoNum);
            msg.data.push_back(m_infoNum >= 8 ? INFO_MODE_PLUS_8 : 0);
            Mode mode = {};
            if (m_device->getModes().size() > m_infoNum)
            {
                mode = m_device->getModes()[m_infoNum];
            }
            switch (m_infoSubNum)
            {
            case 0:
            {
                msg.data[0] |= INFO_NAME;
                auto name = mode.name;
                msg.data.insert(msg.data.end(), name.begin(), name.end());
                msg.data.push_back(0);
                msg.data.insert(msg.data.end(), mode.flags.bytes, mode.flags.bytes + 8);
                break;
            }

            case 1:
                msg.data[0] |= INFO_RAW;
                msg.data.resize(9);
                std::memcpy(msg.data.data() + 1, &mode.min, 4);
                std::memcpy(msg.data.data() + 5, &mode.max, 4);
                break;

            case 2:
                msg.data[0] |= INFO_PCT;
                msg.data.resize(9);
                std::memcpy(msg.data.data() + 1, &mode.PCTmin, 4);
                std::memcpy(msg.data.data() + 5, &mode.PCTmax, 4);
                break;

            case 3:
                msg.data[0] |= INFO_SI;
                msg.data.resize(9);
                std::memcpy(msg.data.data() + 1, &mode.SImin, 4);
                std::memcpy(msg.data.data() + 5, &mode.SImax, 4);
                break;

            case 4:
            {
                msg.data[0] |= INFO_UNITS;
                auto name = mode.unit;
                msg.data.insert(msg.data.end(), name.begin(), name.end());
                msg.data.push_back(0);
                break;
            }

            case 5:
                msg.data[0] |= INFO_MAPPING;
                msg.data.push_back(mode.in.val);
                msg.data.push_back(mode.out.val);
                break;

            case 6:
            {
                if (m_infoNum != 0)
                {
                    okayToWrite = false;
                    break;
                }
                msg.data[0] |= INFO_MODE_COMBOS;
                auto combos = m_device->getModeCombos();
                for (auto combo : combos)
                {
                    msg.data.push_back(combo);
                    msg.data.push_back(combo >> 8);
                }
                break;
            }

            case 7:
                msg.data[0] |= INFO_FORMAT;
                msg.data.push_back(mode.data_sets);
                msg.data.push_back(mode.format);
                msg.data.push_back(mode.figures);
                msg.data.push_back(mode.decimals);
                break;

            default:
                okayToWrite = false;
                m_infoNum++;
                m_infoSubNum = 0xFF;
                if (m_device->getModeCount() <= m_infoNum)
                {
                    sendACK();
                    m_status = STATUS::WAITING_FOR_ACK;
                    m_start = LPF2_GET_TIME();
                }
                break;
            }
            m_infoSubNum++;
            break;
        }
        
        default:
            okayToWrite = false;
            break;
        }
        if (!okayToWrite)
        {
            return;
        }
        m_writer.write(msg);
    }

    void EmulatedPort::reset()
    {
        m_status = STATUS::SENDING_INFO;
        m_start = LPF2_GET_TIME();
        m_infoNum = 0;
        m_infoSubNum = 0;
        m_infoState = InfoState::CMD;
        m_hostType = HostType::NONE;
        m_baud = 115200;
        m_comboPairs.clear();
        changeBaud(m_baud);
        m_serial->discardRxFiFo();
        m_parser.clearBuf();
    }

    void EmulatedPort::changeBaud(uint32_t _baud)
    {
        m_baud = _baud;
        m_serial->setBaudrate(m_baud);
    }

    void EmulatedPort::sendACK(bool NACK)
    {
        m_serial->write(NACK ? BYTE_NACK : BYTE_ACK);
        m_serial->flush();
    }
}; // namespace Lpf2::Local
