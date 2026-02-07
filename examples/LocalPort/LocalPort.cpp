#include <Arduino.h>

#include "Lpf2Local/Lpf2PortLocal.hpp"
#include "Lpf2DeviceDescLib.hpp"
#include "Lpf2DeviceManager.hpp"
#include "Lpf2Devices/ColorSensor.hpp"
#include "Lpf2Devices/BasicMotor.hpp"

#include "device.hpp"

#include "driver/mcpwm.h"

Esp32IO portA_IO(1); // Use UART1
Lpf2PortLocal portA(&portA_IO);
Lpf2DeviceManager deviceManager(portA);

// Port pwm pins
#define PORT_A_PWM_1 21
#define PORT_A_PWM_2 10

// Port ID pins
#define PORT_A_ID_1 15
#define PORT_A_ID_2 16

// mcpwm unit and timer for the port
#define PORT_A_PWM_UNIT mcpwm_unit_t(0)
#define PORT_A_PWM_TIMER mcpwm_timer_t(0)

#define initIOForPort(_port)                                        \
    port##_port##_IO.init(PORT_##_port##_ID_1, PORT_##_port##_ID_2, \
                          PORT_##_port##_PWM_1, PORT_##_port##_PWM_2, PORT_##_port##_PWM_UNIT, PORT_##_port##_PWM_TIMER, 1000);

void setup()
{
    Serial.begin(981200);
    Lpf2DeviceRegistry::registerDefault();
    Lpf2DeviceDescRegistry::registerDefault();

    // Initialize IO before the port, as the port will use the IO to communicate with the device
    initIOForPort(A);
    portA.init();
}

void loop()
{
    vTaskDelay(1);

    deviceManager.update();

    if (deviceManager.device())
    {
        if (auto device = static_cast<TechnicColorSensorControl *>
            (deviceManager.device()->getCapability(TechnicColorSensor::CAP)))
        {
            Serial.print("Color idx: ");
            Serial.println(device->getColorIdx());
        }
        if (auto device = static_cast<BasicMotorControl *>
            (deviceManager.device()->getCapability(BasicMotor::CAP)))
        {
            device->setSpeed(-50);
        }
        else
        {
            // Device isn't a color sensor or a motor (all motors have BasicMotor::CAP)
        }
    }
}