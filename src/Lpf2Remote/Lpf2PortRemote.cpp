#include "Lpf2PortRemote.hpp"
#include "../Lpf2Hub.hpp"

void Lpf2PortRemote::update()
{
    if (m_lastDevType != m_deviceType)
    {
        m_lastDevType = m_deviceType;
        if (m_deviceType != Lpf2DeviceType::UNKNOWNDEVICE)
        {
            setMode(getDefaultMode(m_deviceType));
        }
    }
}

int Lpf2PortRemote::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
{
    if (!m_remote || !deviceConnected())
        return 1;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x51, modeNum};
    payload.insert(payload.end(), data.begin(), data.end());
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
    return 0;
}

bool Lpf2PortRemote::deviceConnected()
{
    if (!m_remote)
        return false;
    return m_deviceType != Lpf2DeviceType::UNKNOWNDEVICE;
}

int Lpf2PortRemote::setMode(uint8_t mode)
{
    if (!m_remote || !deviceConnected())
        return 1;
    uint32_t delta = 1;
    return m_remote->setPortMode(portNum, mode, delta);
}

int Lpf2PortRemote::setModeCombo(uint8_t idx)
{
    return 0;
}

void Lpf2PortRemote::startPower(int8_t pw)
{
    writeData(0, {speedToRaw(pw)});
}

void Lpf2PortRemote::setAccTime(uint16_t accTime, Lpf2AccelerationProfile accProfile)
{
    if (!m_remote || !deviceConnected())
        return;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x05};
    payload.push_back(accTime & 0xFF);
    payload.push_back((accTime >> 8) & 0xFF);
    payload.push_back((uint8_t)accProfile);
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
}

void Lpf2PortRemote::setDecTime(uint16_t decTime, Lpf2AccelerationProfile decProfile)
{
    if (!m_remote || !deviceConnected())
        return;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x06};
    payload.push_back(decTime & 0xFF);
    payload.push_back((decTime >> 8) & 0xFF);
    payload.push_back((uint8_t)decProfile);
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
}

void Lpf2PortRemote::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
{
    if (!m_remote || !deviceConnected())
        return;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x07};
    payload.push_back(speedToRaw(speed));
    payload.push_back(maxPower);
    payload.push_back(useProfile);
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
}

void Lpf2PortRemote::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile)
{
    if (!m_remote || !deviceConnected())
        return;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x05};
    payload.push_back(time & 0xFF);
    payload.push_back((time >> 8) & 0xFF);
    payload.push_back(speedToRaw(speed));
    payload.push_back(maxPower);
    payload.push_back(useProfile);
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
}

void Lpf2PortRemote::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile)
{
    if (!m_remote || !deviceConnected())
        return;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x05};
    payload.push_back(degrees & 0xFF);
    payload.push_back((degrees >> 8) & 0xFF);
    payload.push_back((degrees >> 16) & 0xFF);
    payload.push_back((degrees >> 24) & 0xFF);
    payload.push_back(speedToRaw(speed));
    payload.push_back(maxPower);
    payload.push_back(useProfile);
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
}

void Lpf2PortRemote::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile)
{
    if (!m_remote || !deviceConnected())
        return;
    std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x05};
    payload.push_back(absPos & 0xFF);
    payload.push_back((absPos >> 8) & 0xFF);
    payload.push_back((absPos >> 16) & 0xFF);
    payload.push_back((absPos >> 24) & 0xFF);
    payload.push_back(speedToRaw(speed));
    payload.push_back(maxPower);
    payload.push_back(useProfile);
    m_remote->writeValue(Lpf2MessageType::PORT_OUTPUT_COMMAND, payload);
}

void Lpf2PortRemote::presetEncoder(int32_t pos)
{
    writeData(2, {
        uint8_t((pos >>  0) & 0xFF),
        uint8_t((pos >>  8) & 0xFF),
        uint8_t((pos >> 16) & 0xFF),
        uint8_t((pos >> 14) & 0xFF)
    });
}
