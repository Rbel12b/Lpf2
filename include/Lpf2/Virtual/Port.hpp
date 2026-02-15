#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Port.hpp"
#include "Lpf2/Virtual/Device.hpp"
#include <memory>

class Lpf2PortVirtual : public Lpf2Port
{
public:
    Lpf2PortVirtual() {};

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


    void attachDevice(Lpf2VirtualDevice *device);
    void detachDevice();
private:
    std::unique_ptr<Lpf2VirtualDevice> m_device;
};