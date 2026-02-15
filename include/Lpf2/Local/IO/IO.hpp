#pragma once
#ifndef _LPF2_IO_H_
#define _LPF2_IO_H_

#include "Lpf2/config.hpp"
#include "Lpf2/Local/IO/UART.hpp"

class Lpf2PWM;

class Lpf2IO
{
public:
    virtual ~Lpf2IO() = default;
    virtual Lpf2Uart* getUart() = 0;
    virtual Lpf2PWM* getPWM() = 0;
    virtual bool ready() const = 0;
};

class Lpf2PWM
{
public:
    virtual ~Lpf2PWM() = default;
    virtual void out(uint8_t ch1, uint8_t ch2) = 0;
    void off()
    {
        out(0, 0);
    }
};

#endif