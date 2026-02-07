#include "Lpf2PortRemote.hpp"
#include "../Lpf2Hub.hpp"

void Lpf2PortRemote::update()
{
    if (m_lastDevType != m_deviceType)
    {
        m_lastDevType = m_deviceType;
        setMode(getDefaultMode(m_deviceType));
    }
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

int Lpf2PortRemote::setMode(uint8_t mode)
{
    if (!m_remote)
        return 1;
    uint32_t delta = 1;
    return m_remote->setPortMode(portNum, mode, delta);
}

int Lpf2PortRemote::setModeCombo(uint8_t idx)
{
    return 0;
}
