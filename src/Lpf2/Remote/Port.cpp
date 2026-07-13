/**
 *  Copyright (C) 2026 - Rbel12b
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  */

#include "Lpf2/Remote/Port.hpp"
#include "Lpf2/Hub.hpp"

namespace Lpf2::Remote
{
    void Port::_update()
    {
        if (m_lastDevType != m_deviceType)
        {
            m_rawDataSizeEnsured = false;
            m_lastDevType = m_deviceType;
            if (m_deviceType != DeviceType::UNKNOWNDEVICE)
            {
                setMode(getDefaultMode(m_deviceType));
            }
        }
    }

    int Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
    {
        if (!m_remote || !isDeviceConnected())
            return 1;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x51, modeNum};
        payload.insert(payload.end(), data.begin(), data.end());
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
        return 0;
    }

    bool Port::isDeviceConnected()
    {
        if (!m_remote)
            return false;
        return m_deviceType != DeviceType::UNKNOWNDEVICE;
    }

    int Port::setMode(uint8_t mode, float delta)
    {
        if (!m_remote || !isDeviceConnected())
            return 1;
        storeModeDelta(mode, delta);
        uint32_t bleDelta = (uint32_t)delta;
        return m_remote->setPortMode(m_portNum, mode, bleDelta);
    }

    int Port::setModeCombo(uint8_t idx, const std::vector<float>& deltas)
    {
        if (!m_remote || !isDeviceConnected())
            return 1;

        storeComboDeltas(idx, deltas);

        uint16_t bitmask = m_modeCombos[idx];

        std::vector<uint8_t> nibblePairs;
        for (int m = 0; m < 16; m++)
        {
            if (bitmask & (1u << m))
                nibblePairs.push_back((uint8_t)((m << 4) | 0x00));
        }

        std::vector<uint32_t> bleDeltasPerMode;
        for (size_t i = 0; i < nibblePairs.size(); i++)
            bleDeltasPerMode.push_back((i < deltas.size()) ? (uint32_t)deltas[i] : 1u);

        return m_remote->setPortModeCombo(m_portNum, idx, nibblePairs, bleDeltasPerMode);
    }

    void Port::startPower(int8_t pw)
    {
        writeData(0, {speedToRaw(pw)});
    }

    void Port::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        if (!m_remote || !isDeviceConnected())
            return;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x05};
        payload.push_back(accTime & 0xFF);
        payload.push_back((accTime >> 8) & 0xFF);
        payload.push_back((uint8_t)accProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        if (!m_remote || !isDeviceConnected())
            return;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x06};
        payload.push_back(decTime & 0xFF);
        payload.push_back((decTime >> 8) & 0xFF);
        payload.push_back((uint8_t)decProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        if (!m_remote || !isDeviceConnected())
            return;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x07};
        payload.push_back(speedToRaw(speed));
        payload.push_back(maxPower);
        payload.push_back(useProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_remote || !isDeviceConnected())
            return;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x09};
        payload.push_back(time & 0xFF);
        payload.push_back((time >> 8) & 0xFF);
        payload.push_back(speedToRaw(speed));
        payload.push_back(maxPower);
        payload.push_back(useProfile);
        m_remote->writeValue(MessageType::PORT_OUTPUT_COMMAND, payload);
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_remote || !isDeviceConnected())
            return;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x0B};
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
        if (!m_remote || !isDeviceConnected())
            return;
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x0D};
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