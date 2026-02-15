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