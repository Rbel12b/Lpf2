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

#if defined(LPF2_USE_FREERTOS)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#elif __has_include(<mutex>)
#include <mutex>
#endif // LPF2_USE_FREERTOS

namespace Lpf2::Utils
{
#if defined(LPF2_USE_FREERTOS)

using Mutex = xQueueHandle;
#define LPF2_MUTEX_LOCK(m) xSemaphoreTake(m, portMAX_DELAY)
#define LPF2_MUTEX_UNLOCK(m) xSemaphoreGive(m)
#define LPF2_MUTEX_CREATE() xSemaphoreCreateMutex()
#define LPF2_MUTEX_INVALID nullptr

#elif __has_include(<mutex>)

using Mutex = std::mutex;
#define LPF2_MUTEX_LOCK(m) m.lock()
#define LPF2_MUTEX_UNLOCK(m) m.unlock()

#else

// Dummy mutex implementation
using Mutex = int;
#define LPF2_MUTEX_LOCK(m)
#define LPF2_MUTEX_UNLOCK(m)
#define LPF2_MUTEX_CREATE() 1
#define LPF2_MUTEX_INVALID 0

#endif // LPF2_USE_FREERTOS
    class MutexLock
    {
    public:
        MutexLock(Mutex &mutex)
            : m_mutex(mutex)
        {
            LPF2_MUTEX_LOCK(m_mutex);
        }
        ~MutexLock()
        {
            LPF2_MUTEX_UNLOCK(m_mutex);
        }

    private:
        Mutex &m_mutex;
    };
}; // namespace Lpf2::Utils