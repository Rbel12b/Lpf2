#pragma once

#include "Lpf2/config.hpp"
namespace Lpf2::Utils
{
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
}; // namespace Lpf2::Utils