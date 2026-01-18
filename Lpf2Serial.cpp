#include "Lpf2Serial.h"
#include "Lpf2SerialDef.h"

#include <string>
#include <cstdio>

std::vector<Lpf2Message> Lpf2Parser::update()
{
    std::vector<Lpf2Message> messages;
    while (m_serial->available())
    {
        uint8_t b = m_serial->read();
        buffer.push_back(b);
        m_lastReceivedTime = millis();
    }

    while (buffer.size())
    {
        Lpf2Message message;

        resetChecksum();

        uint8_t b = buffer[0];
        computeChecksum(b);

        message.header = b;
        message.msg = GET_MESSAGE_TYPE(message.header);

        if (message.msg == MESSAGE_SYS)
        {
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
            // if (m_lastReceivedTime - millis() >= 500)
            // {
            //     // Probably corrupted message
            //     buffer.erase(buffer.begin());
            //     log_w("Discarding 1 byte, because message may be corrupted");
            // }
            break;
        }

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
            printMessage(message);
            buffer.erase(buffer.begin());
            continue;
        }

        messages.push_back(message);

        buffer.erase(buffer.begin(), buffer.begin() + message.length + 2);
    }
    return messages;
}

void Lpf2Parser::resetChecksum()
{
    checksum = 0xFF;
}

void Lpf2Parser::computeChecksum(uint8_t b)
{
    checksum ^= b;
}

void Lpf2Parser::printMessage(const Lpf2Message &msg)
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
