#pragma once
#ifndef _LPF2_PORT_REMOTE_H_
#define _LPF2_PORT_REMOTE_H_

#include "config.h"
#include "Lpf2Port.h"
#include <memory>

class Lpf2Hub;

class Lpf2PortRemote : public Lpf2Port
{
public:
    Lpf2PortRemote(Lpf2Hub* remote) : m_remote(remote) {};

    void update() override;

    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
    void setPower(uint8_t pin1, uint8_t pin2) override;
    bool deviceConnected() override;
    int setMode(uint8_t mode) override;
    int setModeCombo(uint8_t idx) override;

protected:
    Lpf2Hub* m_remote;
};

/* --- For internl use only! --- */
class Lpf2PortRemote_internal : public Lpf2PortRemote
{
public:
    Lpf2PortRemote_internal() = delete;

    Lpf2PortRemote_internal(Lpf2Hub* remote, Lpf2PortNum portNum)
     : Lpf2PortRemote(remote)
    {
        setPortNum(portNum);
    };

    void setDevType(Lpf2DeviceType _devType) { m_deviceType = _devType; }
    void setModes(uint8_t _modes) { modes = _modes; }
    void setViews(uint8_t _views) { modes = _views; }
    void setModeCombos(const std::vector<uint16_t> &_modeCombos)
    { 
        modeCombos = _modeCombos;
        modeCombos.resize(8);
    }
    void setCaps(uint8_t _caps) { caps = _caps; }
    void setInModes(uint16_t _inModes) { inModes = _inModes; }
    void setOutModes(uint16_t _outModes) { outModes = _outModes; }
    std::vector<Lpf2Mode>& getModes_mod() { return modeData; }
    void setPortNum(Lpf2PortNum _portNum) { portNum = _portNum; }
};

#endif // _LPF2_PORT_REMOTE_H_