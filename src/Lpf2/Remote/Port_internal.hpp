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
#include "Lpf2/Remote/Port.hpp"
#include <memory>

namespace Lpf2::Remote
{
    class Port_internal : public Port
    {
    public:
        void setCaps(uint8_t _caps) { caps = _caps; }
        void setInModes(uint16_t _inModes) { inModes = _inModes; }
        void setOutModes(uint16_t _outModes) { outModes = _outModes; }
        std::vector<Mode> &getModes_mod() { return modeData; }
        void setPortNum(Lpf2::PortNum _portNum) { portNum = _portNum; }
        Port_internal() = delete;

        Port_internal(Hub *remote, PortNum portNum)
            : Port(remote)
        {
            setPortNum(portNum);
        };

        void setDevType(DeviceType _devType) { m_deviceType = _devType; }
        void setModes(uint8_t _modes) { modes = _modes; }
        void setViews(uint8_t _views) { modes = _views; }
        void setModeCombos(const std::vector<uint16_t> &_modeCombos)
        {
            modeCombos = _modeCombos;
            modeCombos.resize(8);
        }
    };
}; // namespace Lpf2::Remote