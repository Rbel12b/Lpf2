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
#include "Lpf2/Local/IO/UART.hpp"

namespace Lpf2::Local
{
    class PWM;

    class IO
    {
    public:
        virtual ~IO() = default;
        virtual Uart *getUart() = 0;
        virtual PWM *getPWM() = 0;
        virtual bool ready() const = 0;
    };

    class PWM
    {
    public:
        virtual ~PWM() = default;
        virtual void out(uint8_t ch1, uint8_t ch2) = 0;
        void off()
        {
            out(0, 0);
        }
    };
}; // namespace Lpf2::Local