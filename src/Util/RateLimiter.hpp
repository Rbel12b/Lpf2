#pragma once
#ifndef _LPF2_UTIL_RATE_LIMITER_H_
#define _LPF2_UTIL_RATE_LIMITER_H_

#include "../config.hpp"

class RateLimiter
{
public:
    RateLimiter(size_t interval_ms)
    {
        m_interval_ms = interval_ms;
        m_start = LPF2_GET_TIME();
    }
    bool okayToSend()
    {
        return LPF2_GET_TIME() - m_start > m_interval_ms;
    }
    void reset()
    {
        m_start = LPF2_GET_TIME();
    }

private:
    size_t m_start;
    size_t m_interval_ms;
};

#endif