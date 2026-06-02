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
#include <cstring>
#include "Lpf2/Util/Values.hpp"
#include "Lpf2/DeviceDescLib.hpp"

namespace Lpf2::Local
{
    void Port::parseMessage(const Message &msg)
    {
        switch (msg.msg)
        {
        case MESSAGE_CMD:
        {
            m_startRec = LPF2_GET_TIME();
            parseMessageCMD(msg);
            break;
        }
        case MESSAGE_INFO:
        {
            m_startRec = LPF2_GET_TIME();
            parseMessageInfo(msg);
            break;
        }
        case MESSAGE_SYS:
        {
            switch (msg.header)
            {
            case BYTE_ACK:
                if (m_status == STATUS::STATUS_ACK_WAIT)
                {
                    LPF2_LOG_D("ACK received.");
                    m_status = m_new_status;
                }
                else if (m_status == STATUS::STATUS_INFO)
                {
                    LPF2_LOG_D("Info end ACK received.");
                    if (!m_deviceDataReceived)
                    {
                        LPF2_LOG_D("Device data not received, waiting for more info.");
                        break; // we don't know the device yet.
                    }
                    m_status = STATUS::STATUS_ACK_SENDING;
                    m_new_status = STATUS::STATUS_DATA_START;
                }
                break;
            case BYTE_SYNC:
                if (m_status == STATUS::STATUS_SYNC_WAIT)
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
            if (m_status == STATUS::STATUS_DATA_START)
            {
                LPF2_LOG_I("Device connected, type: 0x%02X", (int)m_deviceType);
                m_status = STATUS::STATUS_DATA_RECEIVED;
            }
            m_startRec = LPF2_GET_TIME();
            uint8_t mode = GET_MODE(msg.header);

            if (nextModeExt)
            {
                mode += 8;
                nextModeExt = false;
            }

            if (mode >= m_modeCount)
            {
                break;
            }

            uint8_t size = m_modeData[mode].data_sets * getDataSize(m_modeData[mode].format);

            uint8_t readLen = size;
            if (msg.length < size)
            {
                readLen = msg.length;
            }

            if (m_modeData[mode].rawData.size() < readLen)
            {
                m_modeData[mode].rawData.resize(readLen);
            }

            for (int i = 0; i < readLen; i++)
            {
                m_modeData[mode].rawData[i] = msg.data[i];
            }

            if (m_valueChangeCallback)
            {
                m_valueChangeCallback(mode);
            }
            break;
        }
        default:
        {
            LPF2_LOG_E("Unknown message type: 0x%02X", msg.msg);
        }
        }
    }

    void Port::parseMessageCMD(const Message &msg)
    {
        switch (msg.cmd)
        {
        case CMD_TYPE:
        {
            m_deviceType = (DeviceType)msg.data[0];
            m_deviceConnected = true;
            m_status = STATUS::STATUS_INFO;
            nextModeExt = false;
            if (auto desc = DeviceDescRegistry::instance().getDescriptor(m_deviceType))
            {
                setFromDesc(desc);
                m_deviceDataReceived = true;
                LPF2_LOG_D("Set device details from descriptor lib");
            }
            break;
        }
        case CMD_MODES:
        {
            if (msg.length == 1)
            {
                m_modeCount = m_viewCount = msg.data[0] + 1;
            }
            else if (msg.length == 2)
            {
                m_modeCount = msg.data[0] + 1;
                m_viewCount = msg.data[1] + 1;
            }
            else if (msg.length == 4)
            {
                m_modeCount = msg.data[2] + 1;
                m_viewCount = msg.data[3] + 1;
            }
            m_modeData.resize(m_modeCount);
            break;
        }
        case CMD_SPEED:
        {
            if (msg.length != 4)
            {
                break;
            }
            m_baud = msg.data[0] | ((uint64_t)msg.data[1] << 8) | ((uint64_t)msg.data[2] << 16) | ((uint64_t)msg.data[3] << 24);
            break;
        }
        case CMD_VERSION:
        {
            m_fwVersion = Utils::unPackVersion(msg.data);

            auto hwVersionData = msg.data;
            hwVersionData.erase(hwVersionData.begin(), hwVersionData.begin() + 4);
            m_hwVersion = Utils::unPackVersion(hwVersionData);
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

    void Port::parseMessageInfo(const Message &msg)
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= m_modeCount)
        {
            return;
        }
        if (m_modeData.size() < static_cast<size_t>(m_modeCount))
        {
            m_modeData.resize(m_modeCount);
        }
        switch (msg.data[0] & 0xDF)
        {
        case INFO_NAME:
        {
            if (mode >= m_modeCount)
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
            m_modeData[mode].name = name;
            i++;
            if ((i + 6) <= msg.length && msg.data.size() >= static_cast<size_t>(i + 6))
            {
                std::memcpy(&m_modeData[mode].flags.bytes, msg.data.data() + i, 6);
            }
            break;
        }
        case INFO_RAW:
        {
            if (mode >= m_modeCount || msg.length < 9)
            {
                break;
            }
            if (msg.data.size() >= 9)
            {
                std::memcpy(&m_modeData[mode].min, msg.data.data() + 1, 4);
                std::memcpy(&m_modeData[mode].max, msg.data.data() + 5, 4);
            }
            break;
        }
        case INFO_PCT:
        {
            if (mode >= m_modeCount || msg.length < 9)
            {
                break;
            }
            std::memcpy(&m_modeData[mode].PCTmin, msg.data.data() + 1, 4);
            std::memcpy(&m_modeData[mode].PCTmax, msg.data.data() + 5, 4);
            break;
        }
        case INFO_SI:
        {
            if (mode >= m_modeCount || msg.length < 9)
            {
                break;
            }
            std::memcpy(&m_modeData[mode].SImin, msg.data.data() + 1, 4);
            std::memcpy(&m_modeData[mode].SImax, msg.data.data() + 5, 4);
            break;
        }
        case INFO_UNITS:
        {
            if (mode >= m_modeCount)
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
            m_modeData[mode].unit = unit;
            break;
        }
        case INFO_MAPPING:
        {
            if (mode >= m_modeCount || msg.length < 3)
            {
                break;
            }
            m_modeData[mode].in.val = msg.data[1];
            m_modeData[mode].out.val = msg.data[2];
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
                std::memcpy(&m_modeCombos[i], msg.data.data() + 1 + (i * 2), 2);
                if (m_comboNum == 0 && m_modeCombos[i] == 0)
                {
                    m_comboNum = i;
                }
            }
            break;
        }
        case INFO_FORMAT:
        {
            if (mode >= m_modeCount || msg.length < 5)
            {
                break;
            }
            m_modeData[mode].data_sets = msg.data[1];
            m_modeData[mode].format = msg.data[2];
            m_modeData[mode].figures = msg.data[3];
            m_modeData[mode].decimals = msg.data[4];

            if (mode == m_modeCount - 1)
            {
                m_deviceDataReceived = true;
            }
            break;
        }
        default:
        {
            LPF2_LOG_W("Unknown info: 0x%02X, msg: %s", msg.data[0], Lpf2::Utils::bytes_to_hexString(msg.data).c_str());
            break;
        }
        }
    }
}; // namespace Lpf2::Local