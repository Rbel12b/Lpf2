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

#include <Arduino.h>
#include "Lpf2/HubEmulation.hpp"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/Device.hpp"

#include "driver/mcpwm.h"

Lpf2::HubEmulation hub("Technic Hub", Lpf2::HubType::CONTROL_PLUS_HUB);

// #define USE_LOCAL_PORT 1

#ifdef USE_LOCAL_PORT
#include "../LocalPort/device.hpp"
#include "Lpf2/Local/Port.hpp"
Esp32IO io(1);
Lpf2::Local::Port port(io);
#endif

void setup()
{
    Serial.begin(981200);
    Lpf2::DeviceRegistry::registerDefault();
    Lpf2::DeviceDescRegistry::registerDefault();
    hub.setUseBuiltInDevices(true);
    hub.start();
    hub.setHubBatteryLevel(80);

#ifdef USE_LOCAL_PORT
    io.init(15, 16, 21, 10);
    port.init();
    hub.attachPort(Lpf2::PortNum(Lpf2::ControlPlusHubPort::A), &port);
#endif
}

void loop()
{
    vTaskDelay(1);

    if (Serial.available()) {
        uint8_t c = Serial.read();
        if (c == 0x03) {
            // Ctrl+C received
            ESP.restart();
        }
    }

    hub.update();
#ifdef USE_LOCAL_PORT
    port.update();
#endif
}