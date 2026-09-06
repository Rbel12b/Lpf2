# Logging

The library uses `LPF2_LOG_x` macros defined in `include/Lpf2/log/log.h`.

## Log levels

| Level | Value | Macro | Color |
| --- | --- | --- | --- |
| None | 0 | — | — |
| Error | 1 | `LPF2_LOG_E` | Red |
| Warn | 2 | `LPF2_LOG_W` | Yellow |
| Info | 3 | `LPF2_LOG_I` | Green |
| Debug | 4 | `LPF2_LOG_D` | Cyan |
| Verbose | 5 | `LPF2_LOG_V` | Magenta |

Each level also has a `LPF2_DEBUG_EXPR_x(...)` variant for multi-statement blocks.

## Compile-time level cap

`CONFIG_LPF2_LOG_LEVEL` sets the maximum level compiled into the binary — macros above this level expand to nothing (zero overhead):

```ini
build_flags = -DCONFIG_LPF2_LOG_LEVEL=4   ; compile in up to DEBUG
```

This flag has no effect when runtime filtering is enabled (see below).

## Runtime log level

By default (`CONFIG_LPF2_LOG_LEVEL_RUNTIME=1`), **all** log macros up to verbose are compiled in, but each one checks `lpf2_get_runtime_log_level()` before printing. The default runtime level is **Error (1)**.

Change it at runtime:

```cpp
#include "Lpf2/log/log.h"

lpf2_set_runtime_log_level(LPF2_LOG_LEVEL_DEBUG); // enable debug output
lpf2_set_runtime_log_level(LPF2_LOG_LEVEL_NONE);  // silence everything
```

Level is clamped to 0–5.

To disable runtime filtering and use only the compile-time cap (saves a branch per log call):

```ini
build_flags = -DCONFIG_LPF2_LOG_LEVEL_RUNTIME=0
              -DCONFIG_LPF2_LOG_LEVEL=3        ; only Info and below compiled in
```

## Log output destination

By default logs go to the USB Serial JTAG peripheral (`usb_serial_jtag_write_bytes`). To use Arduino `Serial` instead:

```ini
build_flags = -DLPF2_USE_ARDUINO_SERIAL
```

## Colors

Colored output is enabled by default (`CONFIG_LPF2_LOG_COLORS=1`). Disable with:

```ini
build_flags = -DCONFIG_LPF2_LOG_COLORS=0
```

## Output redirect

`lpf2_log_set_vprintf` replaces the default `vprintf` with any compatible
function — useful for routing logs through MicroPython's USB CDC path or a
custom sink:

```cpp
#include "Lpf2/log/log.h"

// Redirect to a custom sink:
lpf2_log_set_vprintf([](const char *fmt, va_list args) -> int {
    // e.g. forward to mp_hal_stdout_tx_strn or a ring buffer
    return vprintf(fmt, args); // fallback example
});

// Restore default vprintf:
lpf2_log_set_vprintf(nullptr);
```

The function pointer is stored globally; the swap is not thread-safe — call it
once at init time before any log macro fires.

## Routing away from UART0

If UART0 is used for LPF2 communication, ESP-IDF core logs will corrupt the LPF2 data stream. Redirect them to USB CDC:

```ini
build_flags =
    -DCORE_DEBUG_LEVEL=0               ; silence esp-idf logs on UART0
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DCONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=1
    -DCONFIG_ARDUHAL_LOG_COLORS=1
```
