#pragma once
#ifndef _LPF2_PORT_REMOTE_H_
#define _LPF2_PORT_REMOTE_H_

#include "config.hpp"
#include "Lpf2Port.hpp"
#include <memory>

class Lpf2Hub;

class Lpf2PortRemote : public Lpf2Port
{
public:
    Lpf2PortRemote(Lpf2Hub* remote) : m_remote(remote) {};

    void update() override;

    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
    bool deviceConnected() override;
    int setMode(uint8_t mode) override;
    int setModeCombo(uint8_t idx) override;

    void startPower(int8_t pw) override;
    void setAccTime(uint16_t accTime, Lpf2AccelerationProfile accProfile = 1) override;
    void setDecTime(uint16_t decTime, Lpf2AccelerationProfile decProfile = 1) override;
    void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile = 0) override;
    void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile = 0) override;
    void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile = 0) override;
    void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, Lpf2BrakingStyle endState, uint8_t useProfile = 0) override;
    void presetEncoder(int32_t pos) override;

protected:
    Lpf2Hub* m_remote;

    Lpf2DeviceType m_lastDevType = Lpf2DeviceType::UNKNOWNDEVICE;

    static inline const uint8_t startupAndCompletion = 0x10;
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