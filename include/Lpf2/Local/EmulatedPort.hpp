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
#include "Lpf2/Port.hpp"
#include "Lpf2/Virtual/Device.hpp"
#include "Lpf2/Local/Serial.hpp"
#include "Lpf2/Local/SerialDef.hpp"
#include "Lpf2/Util/mutex.hpp"

namespace Lpf2::Local
{
    /**
     * @brief Lpf2::Local::EmulatedPort is the "opposite" of Lpf2::Local::Port, meaning its still a phisical port, but instead of being the master, its a slave (This is for creating a custom Lpf2 device).
     */
    class EmulatedPort
    {
    public:
        EmulatedPort() = delete;
        EmulatedPort(Uart &uart) : m_serial(&uart) {};

        void attachDevice(Lpf2::Virtual::Device &device);
        void detachDevice();

        void init();

        void update();

        bool isHostConnected();

        enum class LPF2_STATUS
        {
            SENDING_INFO,
            WAITING_FOR_ACK,
            SENDING_DATA,
        };

    private:
        void changeBaud(uint32_t baud);
        void sendACK(bool NACK = false);

        void parseMessage(const Message &msg);

        void sendUpdate();

    private:
        LPF2_STATUS m_status = LPF2_STATUS::SENDING_INFO;
        uint32_t m_baud = 2400;
        Uart *m_serial;
        Writer m_writer;
        Parser m_parser;

        Lpf2::Virtual::Device *m_device = nullptr;

        enum class InfoState
        {
            CMD,
            MODE,
        } m_infoState = InfoState::CMD;
        uint8_t m_infoNum = 0; // infoState == MODE -> modeNum
        uint8_t m_infoSubNum = 0;
        size_t m_start = 0;
        uint8_t m_mode = 0;
        std::vector<uint8_t> m_lastModeData;
    };
}; // namespace Lpf2::Local