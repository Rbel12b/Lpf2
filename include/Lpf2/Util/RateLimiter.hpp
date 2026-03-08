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