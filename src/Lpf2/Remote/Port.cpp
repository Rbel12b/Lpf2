#include "Lpf2/Remote/Port.hpp"
#include "Lpf2/Hub.hpp"

namespace Lpf2::Remote
{
    void Port::update()
    {
        if (m_lastDevType != m_deviceType)
        {
            m_lastDevType = m_deviceType;
            if (m_deviceType != DeviceType::UNKNOWNDEVICE)
            {
                setMode(getDefaultMode(m_deviceType));
            }
        }
    }

    int Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
    {
        if (!m_remote || !deviceConnected())
            return 1;
        std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x51, modeNum};
        payload.insert(payload.end(), data.begin(), data.end());
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
        return 0;
    }

    bool Port::deviceConnected()
    {
        if (!m_remote)
            return false;
        return m_deviceType != DeviceType::UNKNOWNDEVICE;
    }

    int Port::setMode(uint8_t mode)
    {
        if (!m_remote || !deviceConnected())
            return 1;
        uint32_t delta = 1;
        return m_remote->setPortMode(portNum, mode, delta);
    }

    int Port::setModeCombo(uint8_t idx)
    {
        return 0;
    }

    void Port::startPower(int8_t pw)
    {
        writeData(0, {speedToRaw(pw)});
    }

    void Port::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        if (!m_remote || !deviceConnected())
            return;
        std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x05};
        payload.push_back(accTime & 0xFF);
        payload.push_back((accTime >> 8) & 0xFF);
        payload.push_back((uint8_t)accProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        if (!m_remote || !deviceConnected())
            return;
        std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x06};
        payload.push_back(decTime & 0xFF);
        payload.push_back((decTime >> 8) & 0xFF);
        payload.push_back((uint8_t)decProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        if (!m_remote || !deviceConnected())
            return;
        std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x07};
        payload.push_back(speedToRaw(speed));
        payload.push_back(maxPower);
        payload.push_back(useProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_remote || !deviceConnected())
            return;
        std::vector<uint8_t> payload = {portNum, startupAndCompletion, 0x05};
        payload.push_back(time & 0xFF);
        payload.push_back((time >> 8) & 0xFF);
        payload.push_back(speedToRaw(speed));
        payload.push_back(maxPower);
        payload.push_back(useProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
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
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
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
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::presetEncoder(int32_t pos)
    {
        writeData(2, {uint8_t((pos >> 0) & 0xFF),
                      uint8_t((pos >> 8) & 0xFF),
                      uint8_t((pos >> 16) & 0xFF),
                      uint8_t((pos >> 14) & 0xFF)});
    }
}; // namespace Lpf2::Remote