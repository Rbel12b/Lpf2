#pragma once
#ifndef _LPF2_VIRTUAL_DEVICE_TRAIN_MOTOR_H_
#define _LPF2_VIRTUAL_DEVICE_TRAIN_MOTOR_H_

#include "../Lpf2VirtualDevice.h"

class Lpf2VirtualTrainMotor : public Lpf2VirtualDevice
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
                "LPF2-TRAIN",
                -100.0f, 100.0f,    // RAW -100–100
                -100.0f, 100.0f,    // PCT -100–100
                -100.0f, 100.0f,    // SI  -100–100
                "\0",               // unit
                0x00, 0x18,         // input mapping, output mapping
                1, DATA8, 4, 0,     // datasets, format, figures, decimals
            }
        };
};

#endif