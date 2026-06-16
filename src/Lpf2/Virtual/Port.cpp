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
    void Port::update()
    {
    }

    int Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
    {
        if (!m_device)
            return 0;
        return m_device->writeData(modeNum, data);
    }

    bool Port::isDeviceConnected()
    {
        return (bool)m_device;
    }

    int Port::setMode(uint8_t mode, float delta)
    {
        if (!m_device)
            return 1;
        storeModeDelta(mode, delta);
        return m_device->setMode(mode);
    }

    int Port::setModeCombo(uint8_t idx, const std::vector<float>& deltas)
    {
        if (!m_device)
            return 1;
        storeComboDeltas(idx, deltas);
        return m_device->setModeCombo(idx);
    }

    void Port::startPower(int8_t pw)
    {
        if (!m_device)
            return;
        m_device->startPower(pw);
    }

    void Port::setAccTime(uint16_t accTime, AccelerationProfile accProfile)
    {
        if (!m_device)
            return;
        m_device->setAccTime(accTime, accProfile);
    }

    void Port::setDecTime(uint16_t decTime, AccelerationProfile decProfile)
    {
        if (!m_device)
            return;
        m_device->setDecTime(decTime, decProfile);
    }

    void Port::startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile)
    {
        if (!m_device)
            return;
        m_device->startSpeed(speed, maxPower, useProfile);
    }

    void Port::startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_device)
            return;
        m_device->startSpeedForTime(time, speed, maxPower, endState, useProfile);
    }

    void Port::startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_device)
            return;
        m_device->startSpeedForDegrees(degrees, speed, maxPower, endState, useProfile);
    }

    void Port::gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile)
    {
        if (!m_device)
            return;
        m_device->gotoAbsPosition(absPos, speed, maxPower, endState, useProfile);
    }

    void Port::presetEncoder(int32_t pos)
    {
        if (!m_device)
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
        m_modeData = m_device->getModes();
        m_modeCombos = m_device->getModeCombos();
        m_capabilities = m_device->getCapabilities();
        m_inModesMask = m_device->getInputModes();
        m_outModesMask = m_device->getOutputModes();
        m_fwVersion = m_device->getFwVersion();
        m_hwVersion = m_device->getHwVersion();

        m_device->setValueChangeCallback(std::bind(&Port::deviceValueChangeCallback, this, std::placeholders::_1));
    }

    void Port::detachDevice()
    {
        m_deviceType = DeviceType::UNKNOWNDEVICE;
        m_device.release();
        m_modeData.clear();
        m_modeCombos.clear();
        m_capabilities = 0;
        m_inModesMask = 0;
        m_outModesMask = 0;
    }

    void Port::deviceValueChangeCallback(uint8_t modeNum)
    {
        m_modeData = m_device->getModes();
        fireValueChangeCallback(modeNum);
    }
}; // namespace Lpf2::Virtual