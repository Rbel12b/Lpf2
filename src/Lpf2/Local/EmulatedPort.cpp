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
    }

    void EmulatedPort::detachDevice()
    {
        m_device = nullptr;
    }

    void EmulatedPort::init()
    {
        if (!m_serial)
        {
            return;
        }
        m_baud = 2400;
        m_serial->uartPinsOn();
        changeBaud(m_baud);
        m_status = LPF2_STATUS::SENDING_INFO;
        m_writer.init(m_serial);
        m_parser.init(m_serial);
    }

    void EmulatedPort::update()
    {
        if (!m_serial || !m_device)
        {
            return;
        }
        
        auto messages = m_parser.update();

        for (const auto &msg : messages)
        {
            LPF2_DEBUG_EXPR_V(
                m_parser.printMessage(msg);
            );
            parseMessage(msg);
        }

        switch (m_status)
        {
        case LPF2_STATUS::SENDING_INFO:
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
                        m_status = LPF2_STATUS::WAITING_FOR_ACK;
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
                        m_status = LPF2_STATUS::WAITING_FOR_ACK;
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
                break;
            }
            m_writer.write(msg);
            break;
        }

        case LPF2_STATUS::SENDING_DATA:
            if (m_device->getModes().size() > m_mode && m_lastModeData == m_device->getModes()[m_mode].rawData)
            {
                sendUpdate();
            }
            [[fallthrough]];
        case LPF2_STATUS::WAITING_FOR_ACK:
            if ((LPF2_GET_TIME() - m_start) > 1000)
            {
                m_status = LPF2_STATUS::SENDING_INFO;
                m_start = LPF2_GET_TIME();
                m_infoNum = 0;
                m_infoSubNum = 0;
                m_infoState = InfoState::CMD;
                m_baud = 2400;
                changeBaud(m_baud);
                m_serial->discardRxFiFo();
                m_parser.clearBuf();
            }
        
        default:
            break;
        }
    }

    bool EmulatedPort::isHostConnected()
    {
        return m_status == LPF2_STATUS::SENDING_DATA;
    }

    void EmulatedPort::parseMessage(const Message &msg)
    {
        m_start = LPF2_GET_TIME();
        if (msg.header == BYTE_ACK && (m_status == LPF2_STATUS::WAITING_FOR_ACK || m_status == LPF2_STATUS::SENDING_INFO))
        {
            changeBaud(m_baud);
            m_status = LPF2_STATUS::SENDING_DATA;
        }
        else if (msg.header == BYTE_NACK && m_status == LPF2_STATUS::SENDING_DATA)
        {
            sendUpdate();
        }
        else if (msg.msg == MESSAGE_CMD && msg.cmd == CMD_SPEED)
        {
            if (msg.length != 4)
            {
                return;
            }
            m_baud = msg.data[0] | ((uint64_t)msg.data[1] << 8) | ((uint64_t)msg.data[2] << 16) | ((uint64_t)msg.data[3] << 24);
            sendACK();
            m_serial->flush();
            changeBaud(m_baud);
            m_serial->flush();
            m_status = LPF2_STATUS::SENDING_DATA;
        }
        else if (msg.msg == MESSAGE_CMD && msg.cmd == CMD_SELECT)
        {
            if (msg.length != 1)
            {
                return;
            }
            m_mode = msg.data[0];
        }
        else
        {
            LPF2_LOG_W("Unknown message: ↓, status: %i", (int)m_status);
            m_parser.printMessage(msg);
        }
    }

    void EmulatedPort::sendUpdate()
    {
        if (m_mode >= 8)
        {
            Message msg;
            msg.msg = MESSAGE_CMD;
            msg.cmd = CMD_EXT_MODE;
            msg.data.push_back(8);
            m_writer.write(msg);
        }
        Mode mode;
        if (m_device->getModes().size() > m_mode)
        {
            mode = m_device->getModes()[m_mode];
        }
        Message msg;
        msg.msg = MESSAGE_DATA;
        msg.cmd = m_mode & 0x07;
        msg.data.insert(msg.data.end(), mode.rawData.begin(), mode.rawData.end());
        m_writer.write(msg);
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
