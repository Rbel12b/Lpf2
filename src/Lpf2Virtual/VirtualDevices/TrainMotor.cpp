#include "TrainMotor.h"
#include "Lpf2Const.h"
#include "Util/hexUtils.h"

Lpf2DeviceType Lpf2VirtualTrainMotor::getDeviceType() const
{
    return Lpf2DeviceType::TRAIN_MOTOR;
}

const std::vector<Lpf2Mode> &Lpf2VirtualTrainMotor::getModes() const
{
    return m_modes;
}

int Lpf2VirtualTrainMotor::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
{
    LPF2_LOG_I("Writing to mode: %i, data: %s", modeNum, bytes_to_hexString(data).c_str());
    return 0;
}

std::vector<uint16_t> Lpf2VirtualTrainMotor::getModeCombos() const
{
    return std::vector<uint16_t>({0x0000U});
}

uint8_t Lpf2VirtualTrainMotor::getModeCount() const
{
    return m_modes.size();
}

uint16_t Lpf2VirtualTrainMotor::getInputModes() const
{
    return 0;
}

uint16_t Lpf2VirtualTrainMotor::getOutputModes() const
{
    return 1;
}

uint8_t Lpf2VirtualTrainMotor::getCapatibilities() const
{
    return 0x01; // Input
}
