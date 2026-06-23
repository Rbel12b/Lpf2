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

#include "Lpf2/Virtual/Port.hpp"
#include <memory>

namespace Lpf2::Virtual
{
    void Port::_update()
    {
    }

    int Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
    {
        if (!m_emulatedDevice)
            return 0;
        return m_emulatedDevice->writeData(modeNum, data);
    }

    bool Port::isDeviceConnected()
    {
        return (bool)m_emulatedDevice;
    }

    int Port::setMode(uint8_t mode, float delta)
    {
        if (!m_emulatedDevice)
            return 1;
        storeModeDelta(mode, delta);
        return m_emulatedDevice->setMode(mode);
    }

    int Port::setModeCombo(uint8_t idx, const std::vector<float>& deltas)
    {
        if (!m_emulatedDevice)
            return 1;
        storeComboDeltas(idx, deltas);
        return m_emulatedDevice->setModeCombo(idx);
    }

    void Port::startPower(int8_t pw)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->startPower(pw);
    }

    void Port::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->setAccTime(accTime, accProfile);
    }

    void Port::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->setDecTime(decTime, decProfile);
    }

    void Port::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->startSpeed(speed, maxPower, useProfile);
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->startSpeedForTime(time, speed, maxPower, endState, useProfile);
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->startSpeedForDegrees(degrees, speed, maxPower, endState, useProfile);
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->gotoAbsPosition(absPos, speed, maxPower, endState, useProfile);
    }

    void Port::presetEncoder(int32_t pos)
    {
        if (!m_emulatedDevice)
            return;
        m_emulatedDevice->presetEncoder(pos);
    }

    void Port::attachDevice(Virtual::Device *device)
    {
        m_emulatedDevice = device;
        swapDevice(device);
        if (!device)
            return;

        m_deviceType = m_emulatedDevice->getDeviceType();
        m_modeData = m_emulatedDevice->getModes();
        m_modeCombos = m_emulatedDevice->getModeCombos();
        m_capabilities = m_emulatedDevice->getCapabilities();
        m_inModesMask = m_emulatedDevice->getInputModes();
        m_outModesMask = m_emulatedDevice->getOutputModes();
        m_fwVersion = m_emulatedDevice->getFwVersion();
        m_hwVersion = m_emulatedDevice->getHwVersion();

        m_emulatedDevice->setValueChangeCallback(std::bind(&Port::deviceValueChangeCallback, this, std::placeholders::_1));
    }

    void Port::detachDevice()
    {
        m_deviceType = DeviceType::UNKNOWNDEVICE;
        m_emulatedDevice = nullptr;
        swapDevice(nullptr);
        m_modeData.clear();
        m_modeCombos.clear();
        m_capabilities = 0;
        m_inModesMask = 0;
        m_outModesMask = 0;
    }

    void Port::deviceValueChangeCallback(uint8_t modeNum)
    {
        if (m_emulatedDevice)
            m_modeData = m_emulatedDevice->getModes();
        fireValueChangeCallback(modeNum);
    }
}; // namespace Lpf2::Virtual