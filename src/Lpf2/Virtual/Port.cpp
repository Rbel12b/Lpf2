#include "Lpf2/Virtual/Port.hpp"
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

bool Lpf2PortVirtual::deviceConnected()
{
    return (bool)m_device;
}

int Lpf2PortVirtual::setMode(uint8_t mode)
{
    if (m_device)
        return 1;
    return m_device->setMode(mode);
}

int Lpf2PortVirtual::setModeCombo(uint8_t idx)
{
    if (m_device)
        return 1;
    return m_device->setModeCombo(idx);
}

void Lpf2PortVirtual::startPower(int8_t pw)
{
    if (m_device)
        return;
    m_device->startPower(pw);
}

void Lpf2PortVirtual::setAccTime(uint16_t accTime, Lpf2AccelerationProfile accProfile)
{
    if (m_device)
        return;
    m_device->setAccTime(accTime, accProfile);
}

void Lpf2PortVirtual::setDecTime(uint16_t decTime, Lpf2AccelerationProfile decProfile)
{
    if (m_device)
        return;
    m_device->setDecTime(decTime, decProfile);
}

void Lpf2PortVirtual::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
{
    if (m_device)
        return;
    m_device->startSpeed(speed, maxPower, useProfile);
}

void Lpf2PortVirtual::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile)
{
    if (m_device)
        return;
    m_device->startSpeedForTime(time, speed, maxPower, endState, useProfile);
}

void Lpf2PortVirtual::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile)
{
    if (m_device)
        return;
    m_device->startSpeedForDegrees(degrees, speed, maxPower, endState, useProfile);
}

void Lpf2PortVirtual::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile)
{
    if (m_device)
        return;
    m_device->gotoAbsPosition(absPos, speed, maxPower, endState, useProfile);
}

void Lpf2PortVirtual::presetEncoder(int32_t pos)
{
    if (m_device)
        return;
    m_device->presetEncoder(pos);
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
