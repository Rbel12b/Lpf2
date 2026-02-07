#pragma once
#ifndef _LPF2_PORT_VIRTUAL_H_
#define _LPF2_PORT_VIRTUAL_H_

#include "config.hpp"
#include "Lpf2Port.hpp"
#include "Lpf2VirtualDevice.hpp"
#include <memory>

class Lpf2PortVirtual : public Lpf2Port
{
public:
    Lpf2PortVirtual() {};

    void update() override;

    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
    void setPower(uint8_t pin1, uint8_t pin2) override;
    bool deviceConnected() override;
    int setMode(uint8_t mode) override;
    int setModeCombo(uint8_t idx) override;


    void attachDevice(Lpf2VirtualDevice *device);
    void detachDevice();
private:
    std::unique_ptr<Lpf2VirtualDevice> m_device;
};

#endif