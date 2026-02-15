#include <Arduino.h>

#include "Lpf2/Local/Port.hpp"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/DeviceManager.hpp"
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"

#include "device.hpp"

#include "driver/mcpwm.h"

Esp32IO portA_IO(1); // Use UART1
Lpf2::Local::Port portA(&portA_IO);
Lpf2::DeviceManager deviceManager(portA);

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
    Lpf2::DeviceRegistry::registerDefault();
    Lpf2::DeviceDescRegistry::registerDefault();

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
        if (auto device = static_cast<Lpf2::Devices::TechnicColorSensorControl *>
            (deviceManager.device()->getCapability(Lpf2::Devices::TechnicColorSensor::CAP)))
        {
            Serial.print("Color idx: ");
            Serial.println(device->getColorIdx());
        }
        if (auto device = static_cast<Lpf2::Devices::BasicMotorControl *>
            (deviceManager.device()->getCapability(Lpf2::Devices::BasicMotor::CAP)))
        {
            device->startPower(-50);
        }
        else
        {
            // Device isn't a color sensor or a motor (all motors have BasicMotor::CAP)
        }
    }
}