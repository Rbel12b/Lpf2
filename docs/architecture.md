# Architecture

## Library Structure

```text
include/Lpf2/
├── config.hpp                # Compile-time configuration
├── LWPConst.hpp              # LWP protocol constants and enums
├── Port.hpp                  # Base Port interface + PortDevice + DeviceSlot
├── Device.hpp                # Base Device interface
├── DeviceDesc.hpp            # Mode/descriptor data structures
├── DeviceDescLib.hpp         # Device descriptor library (built-in mode tables)
├── DeviceFactory.hpp         # DeviceFactory, DeviceRegistry, Lpf2CapabilityRegistry
├── DeviceManager.hpp         # Deprecated shim — Port now owns device lifecycle directly
├── Hub.hpp                   # LEGO Hub BLE control
├── HubEmulation.hpp          # LEGO Hub BLE emulation
├── Devices/                  # Concrete device implementations
│   ├── BasicMotor.hpp
│   ├── EncoderMotor.hpp
│   ├── ColorSensor.hpp        # Technic color sensor
│   ├── ColorDistanceSensor.hpp # Color & Distance sensor (type 37)
│   ├── DistanceSensor.hpp
│   ├── Accelerometer.hpp      # Hub internal accelerometer
│   ├── Gyroscope.hpp          # Hub internal gyroscope
│   └── HubLED.hpp             # Hub internal LED
├── Local/                    # Physical UART port (master/slave)
│   ├── Port.hpp              # Local::Port — master (reads a device)
│   ├── EmulatedPort.hpp      # Local::EmulatedPort — slave (is a device)
│   ├── Serial.hpp            # UART abstraction
│   ├── SerialDef.hpp         # Serial types (Message, Writer, Parser)
│   └── IO/
│       ├── IO.hpp            # IO interface (PWM, ID pins)
│       └── UART.hpp          # UART wrapper
├── Remote/
│   └── Port.hpp              # Remote::Port — hub-connected device
├── Virtual/
│   ├── Device.hpp            # Virtual::Device — descriptor for emulated devices
│   └── Port.hpp              # Virtual::Port — hub-emulation port
├── Util/
│   ├── mutex.hpp
│   ├── RateLimiter.hpp
│   ├── Utils.hpp
│   └── Values.hpp
└── log/
    └── log.h                 # LPF2_LOG_x macros
```

## Key Abstractions

### Port

`Port` (in `Port.hpp`) is the central abstraction. All port types derive from it. It owns the device's mode table, descriptor data, and all motor/sensor commands.

| Class | Role |
| --- | --- |
| `Local::Port` | Master: drives a physical LPF2 device over UART |
| `Local::EmulatedPort` | Slave: presents as a LPF2 device to a host (hub or `Local::Port`) |
| `Remote::Port` | Proxy to a device attached to a LEGO Hub over BLE |
| `Virtual::Port` | Port inside a hub-emulation context |

### Device

`Device` (in `Device.hpp`) is a typed wrapper around a `Port`. `PortDevice` (also in `Port.hpp`) bridges them: it forwards all `Device` interface calls to the underlying `Port`, so `DeviceManager` can manage any port type uniformly.

### DeviceFactory / DeviceRegistry

`DeviceFactory` is a stateless matcher+constructor. `DeviceRegistry` holds up to 32 factories in a static array (no heap). Call `DeviceRegistry::registerDefault()` once at startup to register all built-in device types. Custom devices call `DeviceRegistry::instance().registerFactory(&myFactory)`.

### Lpf2CapabilityRegistry

Capabilities are FNV-1a hashes of a string name, computed at compile time (`constexpr`). No runtime registration is needed — just declare a `static constexpr DeviceCapabilityId CAP = Lpf2CapabilityRegistry::registerCapability("name")` in your device class.

### DeviceDescLib

`DeviceDescLib` stores hard-coded mode tables for known LEGO devices. Calling `DeviceDescRegistry::registerDefault()` makes the library skip the slow UART enumeration sequence by matching device type to a cached descriptor.

### Virtual::Device

`Virtual::Device` is a pure descriptor object — it provides the mode table, capability flags, and data callbacks that `Local::EmulatedPort` sends to the host during handshake. Implement this to define what your custom LPF2 device advertises.

## Data flow — Local::Port (master)

```text
IO::UART ──► Local::Serial ──► Parser ──► Port (mode table, raw values)
                                                 │
                                                 ▼
                                          DeviceManager
                                                 │
                                                 ▼
                                           PortDevice (Device interface)
```

## Data flow — Local::EmulatedPort (slave)

```text
Virtual::Device ──► EmulatedPort ──► Writer ──► Local::Serial ──► IO::UART
                          ▲
                    Parser (host commands — mode select, write data)
```
