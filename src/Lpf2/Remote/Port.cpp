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
    void Port::update()
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
        if (idx >= m_modeCombos.size() || m_modeCombos[idx] == 0)
        {
            LPF2_LOG_W("Invalid combo index: %i", idx);
            return 1;
        }

        storeComboDeltas(idx, deltas);

        uint16_t bitmask = m_modeCombos[idx];

        // Nibble pairs in ascending mode order: high nibble = mode, low nibble = dataset (0)
        std::vector<uint8_t> nibblePairs;
        for (int m = 0; m < 16; m++)
        {
            if (bitmask & (1u << m))
                nibblePairs.push_back((uint8_t)((m << 4) | 0x00));
        }

        // 1. Lock
        m_remote->writeValue(MessageType::PORT_INPUT_FORMAT_SETUP_COMBINEDMODE,
            {m_portNum, 0x02});

        // 2. Per-mode single setup — tells the hub the delta for each mode
        size_t pos = 0;
        for (int m = 0; m < 16; m++)
        {
            if (!(bitmask & (1u << m)))
                continue;
            uint32_t bleDelta = (pos < deltas.size()) ? (uint32_t)deltas[pos] : 1u;
            m_remote->writeValue(MessageType::PORT_INPUT_FORMAT_SETUP_SINGLE,
                {m_portNum, (uint8_t)m,
                 (uint8_t)(bleDelta & 0xFF),
                 (uint8_t)((bleDelta >> 8) & 0xFF),
                 (uint8_t)((bleDelta >> 16) & 0xFF),
                 (uint8_t)((bleDelta >> 24) & 0xFF),
                 0x01}); // notify = true
            pos++;
        }

        // 3. Set Mode & Dataset Combination
        {
            std::vector<uint8_t> payload = {m_portNum, 0x01, idx};
            payload.insert(payload.end(), nibblePairs.begin(), nibblePairs.end());
            m_remote->writeValue(MessageType::PORT_INPUT_FORMAT_SETUP_COMBINEDMODE, payload);
        }

        // 4. Unlock + MultiUpdate Enabled
        m_remote->writeValue(MessageType::PORT_INPUT_FORMAT_SETUP_COMBINEDMODE,
            {m_portNum, 0x03});

        return 0;
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
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x05};
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
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x05};
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
        std::vector<uint8_t> payload = {m_portNum, startupAndCompletion, 0x05};
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