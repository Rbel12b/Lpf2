#include <Arduino.h>

#include "Lpf2Hub.h"
#include "Lpf2DeviceDescLib.h"
#include "Lpf2DeviceManager.h"
#include "Lpf2Devices/ColorSensor.h"
#include "Lpf2Devices/BasicMotor.h"

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
        // Set mode 0 on port A if a device is connected, this is required for the color sensor to start sending data
        if (portA.deviceConnected())
        {
            static bool portASetupDone = false;
            if (!portASetupDone)
            {
                portA.setMode(0);
                portASetupDone = true;
            }
        }

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
                device->setSpeed(-50);
            }
            else
            {
                // Device isn't a color sensor or a motor (all motors have BasicMotor::CAP)
            }
        }
    }
}