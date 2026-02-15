#include <Arduino.h>

#include "Lpf2/Hub.hpp"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/DeviceManager.hpp"
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"
#include "Lpf2/Devices/EncoderMotor.hpp"

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
            vTaskDelay(10); // wait a bit for devices to be registered
            auto &port = *hub.getPort(Lpf2PortNum(Lpf2ControlPlusHubPort::LED));
            port.setRgbColorIdx(Lpf2Color::GREEN);
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
            else if (auto device = static_cast<BasicMotorControl *>
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