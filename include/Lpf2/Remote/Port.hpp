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

#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Port.hpp"
#include <memory>

namespace Lpf2
{
    class Hub;
};

namespace Lpf2::Remote
{
    class Port : public Lpf2::Port
    {
    public:
        Port(Hub *remote) : m_remote(remote) {};

        void update() override;

        int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
        bool isDeviceConnected() override;
        int setMode(uint8_t mode) override;
        int setModeCombo(uint8_t idx) override;

        void startPower(int8_t pw) override;
        void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) override;
        void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) override;
        void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile = 0) override;
        void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void presetEncoder(int32_t pos) override;

    protected:
        Hub *m_remote;

        DeviceType m_lastDevType = DeviceType::UNKNOWNDEVICE;

        static inline const uint8_t startupAndCompletion = 0x10;
    };
}; // namespace Lpf2::Remote