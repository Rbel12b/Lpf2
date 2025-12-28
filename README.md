# Lpf2 Library for esp32

## Overview

Lpf2 is a LEGO PoweredUp Protocol 2 (LPF2) communication library for ESP32-S3 microcontrollers. It provides device communication, serial port management, and device abstraction for LEGO Hub systems.

## Features

- Serial communication over UART
- Device management and detection
- Motor control (BasicMotor)
- Port abstraction for device connectivity
- Logging capabilities
- Configuration management

## Library Structure

```tree
Lpf2/
├── config.h              # Configuration settings
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
- `UART`: Hardware interface for ESP32-S3

### Device Management

- `Lpf2Device`: Base class for connected devices
- `DeviceManager`: Coordinates multiple devices
- `BasicMotor`: Motor device implementation

### Utilities

- `log`: Debug logging system
- `IO`: Input/output abstractions
- `config.h`: Customizable library settings
