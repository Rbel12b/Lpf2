# Lpf2 Library for esp32

## Overview

Lpf2 is a LEGO PoweredUp Protocol 2 (LPF2) communication library for ESP32 microcontrollers. It provides device communication, serial port management, and device abstraction for LEGO Devices. The library supports both local UART communication and remote BLE communication with LEGO Hubs, allowing you to control and interact with LEGO devices in various ways.

Download the library from the platformio registry: [rbel12b/Lpf2](https://registry.platformio.org/libraries/rbel12b/Lpf2)

## Credits

- Thanks to Cornelius Munz for the Legoino [library](https://github.com/corneliusmunz/legoino), and to all its contributors [see Credits in Legoino repo](https://github.com/corneliusmunz/legoino/tree/master?tab=readme-ov-file#credits).
I used the library as a reference, and base for LWP implementation (Hub emulation, and controlling a Hub).
- Thanks to LEGO for releasing the LWP documentation: [LEGO Wireless Protocol](https://lego.github.io/lego-ble-wireless-protocol-docs).
- Thanks to the pybrics team for creating the uart protocol documentation: [LEGO PoweredUp UART Protocol](https://github.com/pybricks/technical-info/blob/master/uart-protocol.md).
- Thanks to h2zero (Ryan Powell) for the great [NimBLE-Arduino library](https://github.com/h2zero/NimBLE-Arduino).

## Disclaimers

LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this project.

## License

This library is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Features

- Serial communication over UART
- Device management and detection
- Port abstraction for device connectivity
- Controlling LEGO Hubs
- Emulating a Hub

## Hardware support

Currently only ESP32 microcontrollers are supported, since the library relies on FreeRTOS features, and the Arduino framework. However it should be possible to port it to other platforms with some adjustments.

These are my build flags for an esp32s3, they might need to be adjusted for other variants:

```ini
; I found that this core works better (latest commit at the time of writing).
platform = https://github.com/platformio/platform-espressif32.git#3c076807e1f55b90799b50b946e76a0508e97778
board = esp32-s3-devkitc-1

build_flags = 
    -std=gnu++2a ; C++ standard
    -Wformat ; format warnings

    -DCORE_DEBUG_LEVEL=0 ; disable esp32 core debug logs, so uart0 can be used to communicate with the devices without interference from the logs
    -DLPF2_LOG_LEVEL=4 ; the library uses the LPF2_LOG_x macros for logging, this sets the log level to debug (4)

    -DARDUINO_USB_CDC_ON_BOOT=1 ; enables the USB CDC serial port on boot -> logs will go to the USB serial port.
    -DCONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=1
    -DCONFIG_ARDUHAL_LOG_COLORS=1 ; prertty logs

    -DBOARD_HAS_PSRAM=1 ; if your board has psram, the library itself doesn't require it.
    -DCONFIG_SPIRAM_USE=1
    -mfix-esp32-psram-cache-issue

build_unflags = -std=gnu++11 ; unflag the default C++11 standard, to avoid conflicts with the C++20 standard used by the library

board_build.arduino.memory_type = qio_opi ; my board qio flash and opi psram, adjust if your board has a different flash or psram type
board_build.psram_type = opi ; psram type
```

## Library Structure

```tree
include/Lpf2/
├── LWPConst.hpp          # Protocol constants
├── Device.hpp            # Base device interface, device factory
├── Port.hpp              # Base Port class
├── DeviceManager.hpp     # Device manager
├── Hub.hpp               # LEGO Hub control
├── HubEmulation.hpp      # LEGO Hub emulation
├── DeviceManager.hpp     # Device manager
├── DeviceDescLib.hpp     # Device descriptor library
├── Devices/              # Device implementations
│   ├── BasicMotor.hpp
│   ├── EncoderMotor.hpp
│   ├── DistanceSensor.hpp
│   └── ColorSensor.hpp
├── Local/                # Local port implementation
│   └── Port.hpp
├── Remote/               # Remote port implementation
│   └── Port.hpp
├── Virtual/              # Virtual (emulated) port implementation
│   ├── Device.hpp
│   └── Port.hpp
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
Lpf2::Hub hub;

// registers the default device descriptors, makes communication faster by using the built-in descriptor library,
// instead of relying on the devices to send the information. Should be called once at startup.
Lpf2::DeviceDescRegistry::registerDefault();
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
auto &portA = *hub.getPort(Lpf2::PortNum(Lpf2::ControlPlusHubPort::A));

portA.setMode(0); // only call it once per device connected, sets the mode (only needed when not using the default mode, wich is mode 0)

portA.update();

if (portA.getDeviceType() == Lpf2::DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR)
{
    portA.setPower(255, 0); // example motor power
}
else if (portA.getDeviceType() == Lpf2::DeviceType::TECHNIC_COLOR_SENSOR)
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
Lpf2::Local::Port portA(&portA_IO); // set up port to use the IO

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
    Lpf2::DeviceDescRegistry::registerDefault();

    // Initialize IO before the port, as the port will use the IO to communicate with the device
    initIOForPort(A);
    portA.init();
}

void loop()
{
    vTaskDelay(1);

    portA.update();

    if (portA.getDeviceType() == Lpf2::DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR)
    {
        portA.setPower(255, 0); // example motor power
    }
    else if (portA.getDeviceType() == Lpf2::DeviceType::TECHNIC_COLOR_SENSOR)
    {
        Serial.print("Color Idx: ");
        Serial.println(portA.getValue(0, 0));
    }
}
```

### Device Manager

```c++
Lpf2::DeviceRegistry::registerDefault(); // should be called once at startup, to register the default device factories.

// port can be any class derived from Lpf2Port
Lpf2::DeviceManager deviceManager(port);
deviceManager.update(); // calls port.update(), checks device type, constructs device, should be called periodically.

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

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"

namespace Lpf2::Devices
{
    class BasicMotorControl
    {
    public:
        virtual ~BasicMotorControl() = default;
        virtual void startPower(int speed) = 0;
    };

    class BasicMotor : public Device, public BasicMotorControl
    {
    public:
        BasicMotor(Port &port) : Device(port) {}

        bool init() override
        {
            startPower(0);
            return true;
        }

        void poll() override
        {
        }

        const char *name() const override
        {
            return "DC Motor (dumb)";
        }

        void startPower(int speed) override;

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("basic_motor");

        static void registerFactory(DeviceRegistry &reg);
    };

    class BasicMotorFactory : public DeviceFactory
    {
    public:
        bool matches(Port &port) const override;

        Device *create(Port &port) const override
        {
            return new BasicMotor(port);
        }

        const char *name() const
        {
            return "Basic Motor Factory";
        }
    };
}; // namespace Lpf2::Devices
```

source:

```c++
#include "Lpf2/Devices/BasicMotor.hpp"

namespace Lpf2::Devices
{
    namespace
    {
        BasicMotorFactory factory;
    }

    void BasicMotor::registerFactory(DeviceRegistry &reg)
    {
        reg.registerFactory(&factory);
    }

    void BasicMotor::startPower(int speed)
    {
        m_port.startPower(speed);
    }

    bool BasicMotor::hasCapability(DeviceCapabilityId id) const
    {
        return id == CAP;
    }

    void *BasicMotor::getCapability(DeviceCapabilityId id)
    {
        if (id == CAP)
            return static_cast<BasicMotorControl *>(this);
        return nullptr;
    }

    bool BasicMotorFactory::matches(Port &port) const
    {
        switch (port.getDeviceType())
        {
        case DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
        case DeviceType::TRAIN_MOTOR:
            return true;
        default:
            break;
        }
        return false;
    }
}; // namespace Lpf2::Devices
```

with replacing the name with the new device's name, adding new functions to the control interface, and most importantly: registering a new capability. A device can support more capabilities, but it must inherit from all the used capability interfaces, like:

```c++
class EncoderMotor : public Device, public EncoderMotorControl, public BasicMotorControl
{};

// then getCapability() becomes

void *EncoderMotor::getCapability(DeviceCapabilityId id)
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
