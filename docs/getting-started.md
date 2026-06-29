# Getting Started

## Installation

Install from the PlatformIO registry:

```ini
lib_deps = rbel12b/Lpf2
```

Or add the git URL directly:

```ini
lib_deps = https://github.com/Rbel12b/Lpf2.git
```

## Recommended Build Flags (ESP32-S3)

```ini
; Tested platform commit — stable at time of writing
platform = https://github.com/platformio/platform-espressif32.git#3c076807e1f55b90799b50b946e76a0508e97778
board = esp32-s3-devkitc-1

build_flags =
    -std=gnu++2a

    ; Silence ESP-IDF core logs so UART0 is usable for LPF2 without interference
    -DCORE_DEBUG_LEVEL=0

    ; Library log level: 0=none 1=error 2=warn 3=info 4=debug
    -DLPF2_LOG_LEVEL=4

    ; Route logs to USB CDC instead of UART0
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DCONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=1
    -DCONFIG_ARDUHAL_LOG_COLORS=1

    -DBOARD_HAS_PSRAM=1
    -DCONFIG_SPIRAM_USE=1
    -mfix-esp32-psram-cache-issue

build_unflags = -std=gnu++11

board_build.arduino.memory_type = qio_opi
board_build.psram_type = opi
```

Adjust `board_build.arduino.memory_type` and `board_build.psram_type` for your specific board variant.

## Required Startup Calls

Call these once before using any port or device:

```cpp
// Cache built-in device descriptors — skips slow UART enumeration for known devices
Lpf2::DeviceDescRegistry::registerDefault();

// Register built-in device factories — required for Port::device() to construct devices
Lpf2::DeviceRegistry::registerDefault();
```

## Examples

| Example | Description |
| --- | --- |
| `LocalPort` | Read a sensor / drive a motor over a physical UART port |
| `RemotePort` | Control a device attached to a LEGO Hub over BLE |
| `RemotePortRtti` | Control a device attached to a LEGO Hub over BLE (Uses `dynamic_cast` instead of capability interface) |
| `EmulatedHub` | Emulate a LEGO Hub that another controller connects to |

Open the library in VS Code and select the example's PlatformIO environment to build.
