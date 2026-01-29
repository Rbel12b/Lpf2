#include "Lpf2DeviceManager.h"

#if defined(LPF2_USE_FREERTOS)
void Lpf2DeviceManager::taskEntryPoint(void *pvParameters)
{
    Lpf2DeviceManager *self = static_cast<Lpf2DeviceManager *>(pvParameters);
    self->loopTask(); // Call actual member function
}

void Lpf2DeviceManager::loopTask()
{
    while (1)
    {
        vTaskDelay(1);
        update();
    }
}
#endif
