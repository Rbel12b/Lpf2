#pragma once
#ifndef _LPF2_UTIL_MUTEX_H_
#define _LPF2_UTIL_MUTEX_H_

#if defined(LPF2_USE_FREERTOS)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

using Mutex = xQueueHandle;
#define LPF2_MUTEX_LOCK(m) xSemaphoreTake(m, portMAX_DELAY)
#define LPF2_MUTEX_UNLOCK(m) xSemaphoreGive(m)
#define LPF2_MUTEX_CREATE() xSemaphoreCreateMutex()
#define LPF2_MUTEX_INVALID nullptr

#elif __has_include(<mutex>)
#include <mutex>

using Mutex = std::mutex;
#define LPF2_MUTEX_LOCK(m) m.lock()
#define LPF2_MUTEX_UNLOCK(m) m.unlock()
#define LPF2_MUTEX_CREATE() std::mutex()
#define LPF2_MUTEX_INVALID std::mutex() // Can't be invalid

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
    MutexLock(Mutex mutex)
        : m_mutex(mutex)
    {
        LPF2_MUTEX_LOCK(m_mutex);
    }
    ~MutexLock()
    {
        LPF2_MUTEX_UNLOCK(m_mutex);
    }

private:
    Mutex m_mutex;
};
#endif // _LPF2_UTIL_MUTEX_H_