#include "Lpf2Port.h"
#include <cstring>

void Lpf2Port::parseMessage(const Lpf2Message &msg)
{
    switch (msg.msg)
    {
    case MESSAGE_CMD:
    {
        m_startRec = millis();
        parseMessageCMD(msg);
        break;
    }
    case MESSAGE_INFO:
    {
        m_startRec = millis();
        parseMessageInfo(msg);
        break;
    }
    case MESSAGE_SYS:
    {
        switch (msg.header)
        {
        case BYTE_ACK:
            if (m_status == LPF2_STATUS::STATUS_ACK_WAIT)
            {
                LPF2_LOG_D("ACK received.");
                m_status = m_new_status;
            }
            else if (m_status == LPF2_STATUS::STATUS_INFO)
            {
                LPF2_LOG_D("Info end ACK received.");
                m_status = LPF2_STATUS::STATUS_ACK_SENDING;
                m_new_status = LPF2_STATUS::STATUS_DATA_START;
            }
            break;
        case BYTE_SYNC:
            if (m_status == LPF2_STATUS::STATUS_SYNC_WAIT)
            {
                LPF2_LOG_D("SYNC received.");
                m_status = m_new_status;
            }
            break;

        default:
            break;
        }
        break;
    }
    case MESSAGE_DATA:
    {
        if (m_status == LPF2_STATUS::STATUS_DATA_START)
        {
            m_status = LPF2_STATUS::STATUS_DATA_RECEIVED;
        }
        m_startRec = millis();
        uint8_t mode = GET_MODE(msg.header);

        if (nextModeExt)
        {
            mode += 8;
            nextModeExt = false;
        }

        if (mode >= modes)
        {
            break;
        }

        uint8_t size = modeData[mode].data_sets * getDataSize(modeData[mode].format);

        uint8_t readLen = size;
        if (msg.length < size)
        {
            readLen = msg.length;
        }

        if (modeData[mode].rawData.size() < readLen)
        {
            modeData[mode].rawData.resize(readLen);
        }

        for (int i = 0; i < readLen; i++)
        {
            modeData[mode].rawData[i] = msg.data[i];
        }
        break;
    }
    default:
    {
        LPF2_LOG_E("Unknown message type: 0x%02X", msg.msg);
    }
    }
}

void Lpf2Port::parseMessageCMD(const Lpf2Message &msg)
{
    switch (msg.cmd)
    {
    case CMD_TYPE:
    {
        m_deviceType = (Lpf2DeviceType)msg.data[0];
        m_deviceConnected = true;
        m_status = LPF2_STATUS::STATUS_INFO;
        nextModeExt = false;
        break;
    }
    case CMD_MODES:
    {
        if (msg.length == 1)
        {
            modes = views = msg.data[1] + 1;
        }
        else if (msg.length == 2)
        {
            modes = msg.data[0] + 1;
            views = msg.data[1] + 1;
        }
        else if (msg.length == 4)
        {
            modes = msg.data[2] + 1;
            views = msg.data[3] + 1;
        }
        modeData.resize(modes);
        break;
    }
    case CMD_SPEED:
    {
        if (msg.length != 4)
        {
            break;
        }
        baud = msg.data[0] | ((uint64_t)msg.data[1] << 8) | ((uint64_t)msg.data[2] << 16) | ((uint64_t)msg.data[3] << 24);
        break;
    }
    case CMD_VERSION:
    {
        break;
    }
    case CMD_EXT_MODE:
    {
        if (msg.data[0])
        {
            nextModeExt = true;
        }
        break;
    }
    default:
    {
        LPF2_LOG_W("Unknown command: 0x%02X", msg.cmd);
        break;
    }
    }
}

void Lpf2Port::parseMessageInfo(const Lpf2Message &msg)
{
    uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
    if (mode >= modes)
    {
        return;
    }
    if (modeData.size() < static_cast<size_t>(modes))
    {
        modeData.resize(modes);
    }
    switch (msg.data[0] & 0xDF)
    {
    case INFO_NAME:
    {
        if (mode >= modes)
        {
            break;
        }
        std::string name;
        int i = 0;
        for (i = 1; i < msg.length; i++)
        {
            if (msg.data[i] == '\0')
            {
                break;
            }
            name += msg.data[i];
        }
        modeData[mode].name = name;
        i++;
        if ((i + 6) <= msg.length && msg.data.size() >= static_cast<size_t>(i + 6))
        {
            std::memcpy(&modeData[mode].flags.bytes, msg.data.data() + i, 6);
        }
        break;
    }
    case INFO_RAW:
    {
        if (mode >= modes || msg.length < 9)
        {
            break;
        }
        if (msg.data.size() >= 9)
        {
            std::memcpy(&modeData[mode].min, msg.data.data() + 1, 4);
            std::memcpy(&modeData[mode].max, msg.data.data() + 5, 4);
        }
        break;
    }
    case INFO_PCT:
    {
        if (mode >= modes || msg.length < 5)
        {
            break;
        }
        float val;
        std::memcpy(&val, msg.data.data() + 1, 4);
        if (val == -100.0f)
        {
            modeData[mode].negativePCT = true;
        }
        break;
    }
    case INFO_SI:
    {
        if (mode >= modes || msg.length < 9)
        {
            break;
        }
        std::memcpy(&modeData[mode].SImin, msg.data.data() + 1, 4);
        std::memcpy(&modeData[mode].SImax, msg.data.data() + 5, 4);
        break;
    }
    case INFO_UNITS:
    {
        if (mode >= modes)
        {
            break;
        }
        std::string unit;
        int i = 0;
        for (i = 1; i < msg.length; i++)
        {
            if (msg.data[i] == '\0')
            {
                break;
            }
            unit += msg.data[i];
        }
        modeData[mode].unit = unit;
        break;
    }
    case INFO_MAPPING:
    {
        if (mode >= modes || msg.length < 3)
        {
            break;
        }
        modeData[mode].in.val = msg.data[1];
        modeData[mode].out.val = msg.data[2];
        break;
    }
    case INFO_MODE_COMBOS:
    {
        uint8_t num = (msg.length - 1) / 2;
        if (num > 15)
        {
            break;
        }
        for (int i = 0; i < num; i++)
        {
            std::memcpy(&modeCombos[i], msg.data.data() + 1 + (i * 2), 2);
            if (comboNum == 0 && modeCombos[num] == 0)
            {
                comboNum = i;
            }
        }
    }
    case INFO_FORMAT:
    {
        if (mode >= modes || msg.length < 5)
        {
            break;
        }
        modeData[mode].data_sets = msg.data[1];
        modeData[mode].format = msg.data[2];
        modeData[mode].figures = msg.data[3];
        modeData[mode].decimals = msg.data[4];
        break;
    }
    default:
    {
        LPF2_LOG_W("Unknown info: 0x%02X", msg.data[0]);
        break;
    }
    }
}
