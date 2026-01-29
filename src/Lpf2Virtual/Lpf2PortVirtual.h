#pragma once
#ifndef _LPF2_PORT_VIRTUAL_H_
#define _LPF2_PORT_VIRTUAL_H_

#include "config.h"
#include "Lpf2Port.h"
#include "Util/mutex.h"

class Lpf2PortVirtual : public Lpf2Port
{
public:
    Lpf2PortVirtual() {};

    void update() override;

    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
    void setPower(uint8_t pin1, uint8_t pin2) override;
    bool deviceConnected() override;
};

#endif