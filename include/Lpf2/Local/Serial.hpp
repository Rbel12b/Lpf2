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

#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Local/IO/IO.hpp"
#include <vector>

namespace Lpf2::Local
{
    struct Message
    {
        uint8_t header = 0;
        uint8_t length = 0;
        std::vector<uint8_t> data;
        uint8_t checksum = 0;
        uint8_t msg = 0;
        uint8_t cmd = 0;
        bool system = false;
    };

    class Parser
    {
    public:
        Parser() {};

        void init(Uart *serial)
        {
            m_serial = serial;
        }

        std::vector<Message> update();
        static void printMessage(const Message &msg);

    private:
        void resetChecksum();
        void computeChecksum(uint8_t b);
        uint8_t getChecksum() { return checksum; }

    private:
        Uart *m_serial = nullptr;
        std::vector<uint8_t> buffer;
        uint8_t checksum;
        uint32_t m_lastReceivedTime = 0;
    };
}; // namespace Lpf2::Local