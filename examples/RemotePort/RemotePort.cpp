#include <Arduino.h>

#include "Lpf2Hub.hpp"
#include "Lpf2DeviceDescLib.hpp"
#include "Lpf2DeviceManager.hpp"
#include "Lpf2Devices/ColorSensor.hpp"
#include "Lpf2Devices/BasicMotor.hpp"

Lpf2Hub hub;

void setup()
{
    Serial.begin(981200);
    Lpf2DeviceRegistry::registerDefault();
    Lpf2DeviceDescRegistry::registerDefault();
    hub.init();
}

void loop()
{
    vTaskDelay(1);

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
        }
        else
        {
            Serial.println("Failed to connect to HUB");
        }
    }

    if (hub.isConnected())
    {
        hub.update();

        auto &portA = *hub.getPort(Lpf2PortNum(Lpf2ControlPlusHubPort::A));

        static Lpf2DeviceManager portADeviceManager(portA);
        portADeviceManager.update();

        if (portADeviceManager.device())
        {
            if (auto device = static_cast<TechnicColorSensorControl *>
                (portADeviceManager.device()->getCapability(TechnicColorSensor::CAP)))
            {
                Serial.print("Color idx: ");
                Serial.println(device->getColorIdx());
            }
            if (auto device = static_cast<BasicMotorControl *>
                (portADeviceManager.device()->getCapability(BasicMotor::CAP)))
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