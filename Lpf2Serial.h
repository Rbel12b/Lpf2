#pragma once
#ifndef _LPF2_SERIAL_H_
#define _LPF2_SERIAL_H_

#include "config.h"
#include "IO/IO.h"
#include <vector>

struct Lpf2Message
{
    uint8_t header;
    uint8_t length;
    std::vector<uint8_t> data;
    uint8_t checksum;
    uint8_t msg;
    uint8_t cmd;
    bool system = false;
};

class Lpf2Parser
{
public:
    Lpf2Parser(Lpf2Uart *serial) : m_serial(serial) {}

    std::vector<Lpf2Message> update();
    static void printMessage(const Lpf2Message &msg);

private:
    void resetChecksum();
    void computeChecksum(uint8_t b);
    uint8_t getChecksum() { return checksum; }

private:
    Lpf2Uart *m_serial;
    std::vector<uint8_t> buffer;
    uint8_t checksum;
    uint32_t m_lastReceivedTime = 0;
};

#endif