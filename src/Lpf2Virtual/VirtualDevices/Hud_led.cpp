#include "Hud_led.h"
#include "Lpf2Const.h"
#include "Util/hexUtils.h"

Lpf2DeviceType Lpf2VirtualHubLed::getDeviceType() const
{
    return Lpf2DeviceType::HUB_LED;
}

const std::vector<Lpf2Mode> &Lpf2VirtualHubLed::getModes() const
{
    return m_modes;
}

int Lpf2VirtualHubLed::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
{
    LPF2_LOG_I("Writing to mode: %i, data: %s", modeNum, bytes_to_hexString(data).c_str());
    return 0;
}

std::vector<uint16_t> Lpf2VirtualHubLed::getModeCombos() const
{
    return std::vector<uint16_t>({0x0000U});
}

uint8_t Lpf2VirtualHubLed::getModeCount() const
{
    return m_modes.size();
}

uint16_t Lpf2VirtualHubLed::getInputModes() const
{
    return 0;
}

uint16_t Lpf2VirtualHubLed::getOutputModes() const
{
    return 3;
}

uint8_t Lpf2VirtualHubLed::getCapatibilities() const
{
    return 0x01; // Input
}
