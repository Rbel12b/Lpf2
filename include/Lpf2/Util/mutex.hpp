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