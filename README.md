# Lpf2 Library for esp32

## Overview

Lpf2 is a LEGO PoweredUp Protocol 2 (LPF2) communication library for ESP32-S3 microcontrollers. It provides device communication, serial port management, and device abstraction for LEGO Devices.

## Credits

- Thanks to Cornelius Munz for the Legoino [library](https://github.com/corneliusmunz/legoino), and to all its contributors [see Credits in Legoino repo](https://github.com/corneliusmunz/legoino/tree/master?tab=readme-ov-file#credits).
I used the library as a reference, and base for LWP implementation (Hub emulation, and controlling a Hub).
- Thanks to LEGO for releasing the LWP documentation: [LEGO Wireless Protocol](https://lego.github.io/lego-ble-wireless-protocol-docs).
- Thanks to the pybrics team for creating the uart protocol documentation: [LEGO PoweredUp UART Protocol](https://github.com/pybricks/technical-info/blob/master/uart-protocol.md).
- Thanks to @h2zero for the great [NimBLE-Arduino library](https://github.com/h2zero/NimBLE-Arduino).

## Features

- Serial communication over UART
- Device management and detection
- Port abstraction for device connectivity
- Logging capabilities
- Configuration management

## Library Structure

```tree
Lpf2/
├── config.h             # Configuration settings
├── Lpf2Const.h          # Protocol constants
├── Lpf2Device.h         # Base device interface
├── Lpf2Port.h/cpp       # Port management
├── Lpf2Serial.h/cpp     # Serial communication
├── Lpf2SerialDef.h      # Serial definitions
├── IO/
│   └── IO.h             # Input/Output abstractions
├── log/
│   ├── log.h/cpp        # Logging utilities
├── Lpf2Devices/         # Device implementations
│   ├── BasicMotor.h/cpp
│   └── DistanceSensor.h/cpp
├── Manager/
│   └── DeviceManager.h  # Device management
└── UART/
    └── UART.h           # UART interface
```

## Key Components

### Serial Communication

- `Lpf2Serial`: Handles LPF2 protocol communication
- `Lpf2Port`: Manages individual communication ports
- `UART`: Base class for any hardware

### Device Management

- `Lpf2Device`: Base class for connected devices
- `DeviceManager`: Coordinates multiple devices
- `BasicMotor`: Motor device implementation

### Utilities

- `log`: Debug logging system
- `IO`: Input/output abstractions
- `config.h`: Customizable library settings
