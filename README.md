# Lpf2 Library for esp32

## Overview

Lpf2 is a LEGO PoweredUp Protocol 2 (LPF2) communication library for ESP32 microcontrollers. It provides device communication, serial port management, and device abstraction for LEGO Devices.

## Credits

- Thanks to Cornelius Munz for the Legoino [library](https://github.com/corneliusmunz/legoino), and to all its contributors [see Credits in Legoino repo](https://github.com/corneliusmunz/legoino/tree/master?tab=readme-ov-file#credits).
I used the library as a reference, and base for LWP implementation (Hub emulation, and controlling a Hub).
- Thanks to LEGO for releasing the LWP documentation: [LEGO Wireless Protocol](https://lego.github.io/lego-ble-wireless-protocol-docs).
- Thanks to the pybrics team for creating the uart protocol documentation: [LEGO PoweredUp UART Protocol](https://github.com/pybricks/technical-info/blob/master/uart-protocol.md).
- Thanks to h2zero (Ryan Powell) for the great [NimBLE-Arduino library](https://github.com/h2zero/NimBLE-Arduino).

## License

This library is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Features

- Serial communication over UART
- Device management and detection
- Port abstraction for device connectivity
- Controlling LEGO Hubs
- Emulating a Hub

## Library Structure

```tree
src/
├── Lpf2Const.h             # Protocol constants
├── Lpf2Device.h            # Base device interface, device factory
├── Lpf2Port.h              # Base Port class
├── Lpf2DeviceManager.h     # Device manager
├── Lpf2Hub.h               # LEGO Hub control
├── Lpf2HubEmulation.h      # LEGO Hub emulation
├── Lpf2DeviceManager.h     # Device manager
├── Lpf2DeviceDescLib.h     # Device descriptor library
├── Lpf2Devices/            # Device implementations
│   ├── BasicMotor.h
│   ├── EncoderMotor.h
│   ├── DistanceSensor.h
│   └── ColorSensor.h
├── Lpf2Local/              # Local port implementation
│   └── Lpf2PortLocal.h
├── Lpf2Remote/             # Remote port implementation
│   └── Lpf2PortRemote.h
├── Lpf2Virtual/            # Virtual (emulated) port implementation
│   ├── Lpf2VirtualDevice.h
│   └── Lpf2PortVirtual.h
```

## Functionalities

### Device Communication

- UART-based serial communication with LEGO devices
- LPF2 protocol implementation for reliable device messaging
- Automatic device detection and enumeration

### Device Management

- Dynamic device discovery and registration
- Device descriptor library for protocol information
- Factory pattern for creating device instances
- Support for multiple device types (motors, sensors, etc.)

### Port Abstraction

- Unified interface for different port types
- Local port support for direct UART connections
- Remote port support for hub-connected devices
- Virtual port support for hub emulation

### Hub Control

- BLE connectivity to LEGO Hubs
- Command execution and device control
- Real-time device status monitoring

### Hub Emulation

- Emulate a LEGO Hub
- Respond to hub protocol commands
- Support for virtual device definitions

### Device Support

- Basic Motors with power control
- Encoder Motors with position feedback
- Color Sensors for color detection
- Distance Sensors for proximity measurement

## How-tos

### Use an example

Open the library in VSCode and select the example's platformio enviroment.

### Controlling a hub

Examples:

- [Remote port](examples/RemotePort/RemotePort.cpp)

#### Connecting

```c++
// registers the default device descriptors, makes communication faster by using the built-in descriptor library,
// instead of relying on the devices to send the information. Should be called once at startup.
Lpf2DeviceDescRegistry::registerDefault();
vTaskDelay(1); // for resetting the wdt in a loop

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
    // Do anything with the hub.
}
```

#### Using a remote port

```c++
// get the A port from a hub
auto &portA = *hub.getPort(Lpf2PortNum(Lpf2ControlPlusHubPort::A));

portA.setMode(0); // only call it once per device connected, sets the mode (only needed when not using the default mode, wich is mode 0)

portA.update();

if (portA.getDeviceType() == Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR)
{
    portA.setPower(255, 0); // example motor power
}
else if (portA.getDeviceType() == Lpf2DeviceType::TECHNIC_COLOR_SENSOR)
{
    Serial.print("Color Idx: ");
    Serial.println(portA.getValue(0, 0));
}
```

it is also possible to [use a device manager](#device-manager) with a remote port

### Local ports

Examples:

- [Local port](examples/LocalPort/LocalPort.cpp)

#### Using a local port

```c++
// Esp32IO is from an example, see examples/LocalPort/device.h
Esp32IO portA_IO(1); // Use UART1
Lpf2PortLocal portA(&portA_IO); // set up port to use the IO

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
    Lpf2DeviceDescRegistry::registerDefault();

    // Initialize IO before the port, as the port will use the IO to communicate with the device
    initIOForPort(A);
    portA.init();
}

void loop()
{
    vTaskDelay(1);

    portA.update();

    if (portA.getDeviceType() == Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR)
    {
        portA.setPower(255, 0); // example motor power
    }
    else if (portA.getDeviceType() == Lpf2DeviceType::TECHNIC_COLOR_SENSOR)
    {
        Serial.print("Color Idx: ");
        Serial.println(portA.getValue(0, 0));
    }
}
```

### Device Manager

```c++
Lpf2DeviceRegistry::registerDefault(); // should be called once at startup, to register the default device factories.

// port can be any class derived from Lpf2Port
Lpf2DeviceManager deviceManager(port);
deviceManager.update(); // calls port.update(), checks device type, constructs device, should be called periodically.

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
```

### Adding a new device

Adding a new device to the device manager is easy, you just need to create a header and a source file like this:

header:

```c++
#pragma once
#ifndef _LPF2_BASIC_MOTOR_H_
#define _LPF2_BASIC_MOTOR_H_

#include "../config.h"
#include "../Lpf2Device.h"

class BasicMotorControl
{
public:
    virtual ~BasicMotorControl() = default;
    virtual void setSpeed(int speed) = 0;
};

class BasicMotor : public Lpf2Device, public BasicMotorControl
{
public:
    BasicMotor(Lpf2Port &port) : Lpf2Device(port) {}

    bool init() override
    {
        setSpeed(0);
        return true;
    }

    void poll() override
    {
    }

    const char *name() const override
    {
        return "DC Motor (dumb)";
    }

    void setSpeed(int speed) override;

    bool hasCapability(Lpf2DeviceCapabilityId id) const override;
    void *getCapability(Lpf2DeviceCapabilityId id) override;

    inline static const Lpf2DeviceCapabilityId CAP =
        Lpf2CapabilityRegistry::registerCapability("basic_motor");

    static void registerFactory(Lpf2DeviceRegistry& reg);
};

class BasicMotorFactory : public Lpf2DeviceFactory
{
public:
    bool matches(Lpf2Port &port) const override;

    Lpf2Device *create(Lpf2Port &port) const override
    {
        return new BasicMotor(port);
    }

    const char *name() const
    {
        return "Basic Motor Factory";
    }
};

#endif
```

source:

```c++
#include "BasicMotor.h"

namespace
{
    BasicMotorFactory factory;
}

void BasicMotor::registerFactory(Lpf2DeviceRegistry& reg)
{
    reg.registerFactory(&factory);
}

void BasicMotor::setSpeed(int speed)
{
    assert(this);
    assert(((uintptr_t)this & 0x3) == 0);
    assert(m_port.deviceConnected()); 
    bool forward = speed >= 0;
    speed = std::abs(speed);
    if (speed > 100)
        speed = 100;

    uint8_t pwr2 = speed * 0xFF / 100;
    uint8_t pwr1 = 0;
    if (!forward)
        std::swap(pwr1, pwr2);

    m_port.setPower(pwr1, pwr2);
}

bool BasicMotor::hasCapability(Lpf2DeviceCapabilityId id) const
{
    return id == CAP;
}

void *BasicMotor::getCapability(Lpf2DeviceCapabilityId id)
{
    if (id == CAP)
        return static_cast<BasicMotorControl *>(this);
    return nullptr;
}

bool BasicMotorFactory::matches(Lpf2Port &port) const
{
    switch (port.getDeviceType())
    {
    case Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
    case Lpf2DeviceType::TRAIN_MOTOR:
        return true;
    default:
        break;
    }
    return false;
}
```

with replacing the name with the new device's name, adding new functions to the control interface, and most importantly: registering a new capability. A device can support more capabilities, but it must inherit from all the used capability interfaces, like:

```c++
class EncoderMotor : public Lpf2Device, public EncoderMotorControl, public BasicMotorControl
{};

// then getCapability() becomes

void *EncoderMotor::getCapability(Lpf2DeviceCapabilityId id)
{
    if (id == CAP)
        return static_cast<EncoderMotorControl *>(this);
    if (id == BasicMotor::CAP)
        return static_cast<BasicMotorControl *>(this);
    return nullptr;
}

```

## Logging

The library uses the LPF2_LOG_x macros for logging

```text
build_flags = -DLPF2_LOG_LEVEL=4 ; used to set the log level (4 -> debug)
```
