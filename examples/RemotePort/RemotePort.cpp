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

#include "Lpf2/Hub.hpp"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/DeviceManager.hpp"
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"
#include "Lpf2/Devices/EncoderMotor.hpp"

Lpf2::Hub hub;

void setup()
{
    Serial.begin(981200);
    Lpf2::DeviceRegistry::registerDefault();
    Lpf2::DeviceDescRegistry::registerDefault();
    hub.init();
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

    if (!hub.isConnected() && !hub.isConnecting())
    {
        hub.init();
        vTaskDelay(500);
    }

    if (hub.isConnecting())
    {
        hub.connectHub();
        if (hub.isConnected())
        {
            Serial.println("Connected to HUB");
            vTaskDelay(10); // wait a bit for devices to be registered
            auto &port = *hub.getPort(Lpf2::PortNum(Lpf2::ControlPlusHubPort::LED));
            port.setRgbColorIdx(Lpf2::ColorIDX::GREEN);
        }
        else
        {
            Serial.println("Failed to connect to HUB");
        }
    }

    if (hub.isConnected())
    {
        hub.update();

        auto &portA = *hub.getPort(Lpf2::PortNum(Lpf2::ControlPlusHubPort::A));

        static Lpf2::DeviceManager portADeviceManager(portA);
        portADeviceManager.update();

        if (portADeviceManager.device())
        {
            if (auto device = static_cast<Lpf2::Devices::TechnicColorSensorControl *>
                (portADeviceManager.device()->getCapability(Lpf2::Devices::TechnicColorSensor::CAP)))
            {
                Serial.print("Color idx: ");
                Serial.println(device->getColorIdx());
            }
            else if (auto device = static_cast<Lpf2::Devices::BasicMotorControl *>
                (portADeviceManager.device()->getCapability(Lpf2::Devices::BasicMotor::CAP)))
            {
                device->startPower(-50);
            }
            else
            {
                // Device isn't a color sensor or a motor (all motors have BasicMotor::CAP)
            }
            vTaskDelay(100);
        }
    }
}