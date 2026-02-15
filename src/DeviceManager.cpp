#include "Lpf2/DeviceManager.hpp"

#if defined(LPF2_USE_FREERTOS)
void DeviceManager::taskEntryPoint(void *pvParameters)
{
    DeviceManager *self = static_cast<DeviceManager *>(pvParameters);
    self->loopTask(); // Call actual member function
}

void DeviceManager::loopTask()
{
    while (1)
    {
        vTaskDelay(1);
        update();
    }
}
#endif
