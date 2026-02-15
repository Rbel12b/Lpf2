#include "Lpf2/Virtual/Port.hpp"
#include <memory>

namespace Lpf2::Virtual
{
    void Port::update()
    {
    }

    int Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
    {
        if (!m_device)
            return 0;
        return m_device->writeData(modeNum, data);
    }

    bool Port::deviceConnected()
    {
        return (bool)m_device;
    }

    int Port::setMode(uint8_t mode)
    {
        if (m_device)
            return 1;
        return m_device->setMode(mode);
    }

    int Port::setModeCombo(uint8_t idx)
    {
        if (m_device)
            return 1;
        return m_device->setModeCombo(idx);
    }

    void Port::startPower(int8_t pw)
    {
        if (m_device)
            return;
        m_device->startPower(pw);
    }

    void Port::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        if (m_device)
            return;
        m_device->setAccTime(accTime, accProfile);
    }

    void Port::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        if (m_device)
            return;
        m_device->setDecTime(decTime, decProfile);
    }

    void Port::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        if (m_device)
            return;
        m_device->startSpeed(speed, maxPower, useProfile);
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (m_device)
            return;
        m_device->startSpeedForTime(time, speed, maxPower, endState, useProfile);
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (m_device)
            return;
        m_device->startSpeedForDegrees(degrees, speed, maxPower, endState, useProfile);
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (m_device)
            return;
        m_device->gotoAbsPosition(absPos, speed, maxPower, endState, useProfile);
    }

    void Port::presetEncoder(int32_t pos)
    {
        if (m_device)
            return;
        m_device->presetEncoder(pos);
    }

    void Port::attachDevice(Virtual::Device *device)
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

    void Port::detachDevice()
    {
        m_deviceType = DeviceType::UNKNOWNDEVICE;
        m_device.release();
        modeData.clear();
        modeCombos.clear();
        caps = 0;
        inModes = 0;
        outModes = 0;
    }
}; // namespace Lpf2::Virtual