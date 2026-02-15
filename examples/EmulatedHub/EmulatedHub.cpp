#include <Arduino.h>

#include "Lpf2/HubEmulation.hpp"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/Device.hpp"

#include "driver/mcpwm.h"

Lpf2::HubEmulation hub("Technic Hub", Lpf2::HubType::CONTROL_PLUS_HUB);

void setup()
{
    Serial.begin(981200);
    Lpf2::DeviceRegistry::registerDefault();
    Lpf2::DeviceDescRegistry::registerDefault();
    hub.setUseBuiltInDevices(true);
    hub.start();
    hub.setHubBatteryLevel(80);
}

void loop()
{
    vTaskDelay(1);

    hub.update();
}