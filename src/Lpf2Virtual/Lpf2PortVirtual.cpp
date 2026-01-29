#include "Lpf2PortVirtual.h"
#include <memory>

void Lpf2PortVirtual::update()
{
}

int Lpf2PortVirtual::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
{
    if (!m_device)
        return 0;
    return m_device->writeData(modeNum, data);
}

void Lpf2PortVirtual::setPower(uint8_t pin1, uint8_t pin2)
{
    if (m_device)
        return;
    m_device->setPower(pin1, pin2);
}

bool Lpf2PortVirtual::deviceConnected()
{
    return (bool)m_device;
}

void Lpf2PortVirtual::attachDevice(Lpf2VirtualDevice *device)
{
    m_device.release();
    if (!device)
        return;
    m_device.reset(device);
    
    m_deviceType = m_device->getDeviceType();
    modeData = m_device->getModes();
    modeCombos = m_device->getModeCombos();
    caps = m_device->getCapatibilities();
    inModes = m_device->getInputModes();
    outModes = m_device->getOutputModes();
}

void Lpf2PortVirtual::detachDevice()
{
    m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
    m_device.release();
    modeData.clear();
    modeCombos.clear();
    caps = 0;
    inModes = 0;
    outModes = 0;
}
