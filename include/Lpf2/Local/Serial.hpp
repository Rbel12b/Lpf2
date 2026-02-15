#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Local/IO/IO.hpp"
#include <vector>

namespace Lpf2::Local
{
    struct Message
    {
        uint8_t header;
        uint8_t length;
        std::vector<uint8_t> data;
        uint8_t checksum;
        uint8_t msg;
        uint8_t cmd;
        bool system = false;
    };

    class Parser
    {
    public:
        Parser(Uart *serial) : m_serial(serial) {}

        std::vector<Message> update();
        static void printMessage(const Message &msg);

    private:
        void resetChecksum();
        void computeChecksum(uint8_t b);
        uint8_t getChecksum() { return checksum; }

    private:
        Uart *m_serial;
        std::vector<uint8_t> buffer;
        uint8_t checksum;
        uint32_t m_lastReceivedTime = 0;
    };
}; // namespace Lpf2::Local