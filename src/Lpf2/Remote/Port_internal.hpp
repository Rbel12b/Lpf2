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