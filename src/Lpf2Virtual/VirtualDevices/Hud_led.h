#pragma once
#ifndef _LPF2_VIRTUAL_DEVICE_HUB_LED_H_
#define _LPF2_VIRTUAL_DEVICE_HUB_LED_H_

#include "../Lpf2VirtualDevice.h"

class Lpf2VirtualHubLed : public Lpf2VirtualDevice
{
public:
    Lpf2DeviceType getDeviceType() const override;
    const std::vector<Lpf2Mode> &getModes() const override;
    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
    void setPower(uint8_t pin1, uint8_t pin2) override;
    std::vector<uint16_t> getModeCombos() const override;
    uint8_t getModeCount() const override;
    uint16_t getInputModes() const override;
    uint16_t getOutputModes() const override;
    uint8_t getCapatibilities() const override;

private:
    std::vector<Lpf2Mode> m_modes =
        {
            {
                "COL O",
                0.0f, 10.0f,   // raw 0-10
                0.0f, 100.0f,  // PCT default
                0.0f, 10.0f,   // SI 0-10
                "\0",          // Unit: \0
                0x00, 0x44,// input mapping: 0, output mapping: mapping2 | discrete
                1, DATA8, 1, 0 // 1 data set, 1 byte, 1 figure, 0 decimals
            },
            {
                "RGB O",
                -100.0f, 100.0f, // raw -100–100
                -100.0f, 100.0f, // PCT -100–100
                -100.0f, 100.0f, // SI -100–100
                "\0",            // unit
                0x00, 0x18,  // input mapping, output mapping
                1, DATA8, 4, 0,  // datasets, format, figures, decimals
            }

        };
};

#endif