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
    Lpf2PortRemote() {};

    void update() override;

    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
    void setPower(uint8_t pin1, uint8_t pin2) override;
    bool deviceConnected() override;

    void setRemote(Lpf2Hub* remote);

    /* --- For internl use only! --- */
    void setDevType(Lpf2DeviceType _devType) { m_deviceType = _devType; }
    /* --- For internl use only! --- */
    void setModes(uint8_t _modes) { modes = _modes; }
    /* --- For internl use only! --- */
    void setViews(uint8_t _views) { modes = _views; }
    /* --- For internl use only! --- */
    void setModeCombos(const std::vector<uint16_t> &_modeCombos)
    { 
        modeCombos = _modeCombos;
        modeCombos.resize(8);
    }
    /* --- For internl use only! --- */
    void setCaps(uint8_t _caps) { caps = _caps; }
    /* --- For internl use only! --- */
    void setInModes(uint16_t _inModes) { inModes = _inModes; }
    /* --- For internl use only! --- */
    void setOutModes(uint16_t _outModes) { outModes = _outModes; }
    /* --- For internl use only! --- */
    std::vector<Lpf2Mode>& getModes_mod() { return modeData; }
private:

    Lpf2Hub* m_remote;
};

#endif // _LPF2_PORT_REMOTE_H_