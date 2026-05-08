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

#include "Lpf2/Local/Serial.hpp"
#include "Lpf2/Local/SerialDef.hpp"
#include "Lpf2/Util/Values.hpp"

#include <string>
#include <cstdio>

namespace Lpf2::Local
{
    std::vector<Message> Parser::update()
    {
        std::vector<Message> messages;
        int available = m_serial->available();
        if (available > 0)
        {
            buffer.resize(buffer.size() + available);
            m_serial->read(buffer.data() + buffer.size() - available, available);
        }
        // else
        // {
        //     LPF2_LOG_D("No bytes received (%i).", available);
        // }

        while (buffer.size())
        {
            Message message;

            resetChecksum();

            uint8_t b = buffer[0];
            computeChecksum(b);

            message.header = b;
            message.msg = GET_MESSAGE_TYPE(message.header);

            if (message.msg == MESSAGE_SYS)
            {
                m_lastReceivedTime = LPF2_GET_TIME();
                message.system = true;
                messages.push_back(message);
                buffer.erase(buffer.begin());
                continue;
            }
            else
            {
                message.system = false;
            }

            message.length = GET_MESSAGE_LENGTH(b);

            if (GET_MESSAGE_TYPE(message.header) == MESSAGE_INFO)
            {
                message.length++; // +1 command byte
            }

            if (buffer.size() < message.length + 2)
            {
                if (LPF2_GET_TIME() - m_lastReceivedTime >= 1000)
                {
                    // Probably corrupted message
                    buffer.erase(buffer.begin());
                    LPF2_LOG_W("Discarding 1 byte, because message may be corrupted");
                    continue;
                }
                break;
            }
            m_lastReceivedTime = LPF2_GET_TIME();

            message.data.clear();
            message.data.reserve(message.length);

            for (int i = 0; i < message.length; i++)
            {
                message.data.push_back(buffer[i + 1]);
                computeChecksum(buffer[i + 1]);
            }

            b = buffer[message.length + 1];

            message.checksum = b;

            message.msg = GET_MESSAGE_TYPE(message.header);
            message.cmd = GET_CMD_COMMAND(message.header);

            if (b != getChecksum())
            {
                LPF2_LOG_W("Checksum mismatch: 0x%02X != 0x%02X", b, getChecksum());
                // printMessage(message);
                buffer.erase(buffer.begin());
                continue;
            }

            messages.push_back(message);

            buffer.erase(buffer.begin(), buffer.begin() + message.length + 2);
        }
        return messages;
    }

    void Parser::resetChecksum()
    {
        checksum = 0xFF;
    }

    void Parser::computeChecksum(uint8_t b)
    {
        checksum ^= b;
    }

    void Parser::printMessage(const Message &msg)
    {
        std::string str;
        if (msg.system)
        {
            char buf[32];
            str += "Sys: ";
            switch (msg.header)
            {
            case BYTE_ACK:
                str += "ACK";
                break;
            case BYTE_NACK:
                str += "NACK";
                break;
            case BYTE_SYNC:
                str += "SYNC";
                break;
            default:
                sprintf(buf, "Unknown (0x%02X)", msg.header);
                str += buf;
                break;
            }
            LPF2_LOG_I("%s", str.c_str());
            return;
        }

        char buf[256];
        sprintf(buf, "Header: 0x%02X, Length: %d, MsgType: 0x%02X, Cmd: 0x%02X, Data: ", msg.header, msg.length, msg.msg, msg.cmd);
        str += buf;

        for (uint8_t b : msg.data)
        {
            sprintf(buf, "0x%02X ", b);
            str += buf;
        }
        sprintf(buf, ", C: 0x%02X", msg.checksum);
        str += buf;
        LPF2_LOG_I("%s", str.c_str());
    }

    void Parser::clearBuf()
    {
        buffer.clear();
    }

    void Writer::computeChecksum(uint8_t b)
    {
        checksum ^= b;
    }

    void Writer::write(Message &msg)
    {
        checksum = 0xFF;
        uint8_t size = 0;
        uint8_t dataLen = msg.data.size();
        if (msg.msg == MESSAGE_INFO)
        {
            dataLen--;
        }
        while ((1 << size) < dataLen)
        {
            size++;
        }
        if (size > 7)
        {
            size = 7;
        }
        msg.header = (msg.msg & 0xC0) | ((size & 0x07) << 3) | (msg.cmd & 0x07);

        std::vector<uint8_t> data;
        data.reserve((1 << size) + (msg.msg == MESSAGE_INFO ? 3 : 2));
        data.push_back(msg.header);
        computeChecksum(msg.header);
        uint8_t extraByte = msg.msg == MESSAGE_INFO ? 1 : 0;
        for (size_t i = 0; i < ((1 << size) + extraByte); i++)
        {
            uint8_t b = 0;
            if (i < msg.data.size())
            {
                b = msg.data[i];
            }
            else 
            {
                msg.data.push_back(0);
            }
            computeChecksum(b);
            data.push_back(b);
        }
        data.push_back(checksum);
        msg.checksum = checksum;
        msg.length = (1 << size) + extraByte;
        m_serial->write(data.data(), data.size());
        m_serial->flush();
    }
}; // namespace Lpf2::Local
