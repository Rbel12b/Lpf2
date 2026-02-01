#include "Lpf2PortRemote.h"
#include "../Lpf2Hub.h"

void Lpf2PortRemote::update()
{
}

int Lpf2PortRemote::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
{
    return 0;
}

void Lpf2PortRemote::setPower(uint8_t pin1, uint8_t pin2)
{
}

bool Lpf2PortRemote::deviceConnected()
{
    if (!m_remote)
        return false;
    return m_deviceType != Lpf2DeviceType::UNKNOWNDEVICE;
}

void Lpf2PortRemote::setRemote(Lpf2Hub *remote)
{
    m_remote = remote;
}
