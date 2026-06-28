# Changelog

## 2.5.0 — 2026-06-26

Added a new `getSpeed()` and `getAbsPosition()` method to the `EncoderMotorControl` interface, allowing users to retrieve the speed and position measured by the encoder.

## 2.4.3 — 2026-06-26

Fixed LWP commands to use the correct subcommand byte in `Remote::Port`.

## 2.4.2 — 2026-06-26

Added default arguments to the motor control methods in `Port`.

## 2.4.1 — 2026-06-26

Fixed build errors in Battery.cpp when building with the Arduino framework
by adding an arduino framewrk variant of the implementation.

Also fixed Utils::map() to include the implementation.

## 2.4.0 — 2026-06-26

- Motor PID for local encoder motors rewritten as pct-domain control
  with per-motor tuning. Replaces the single shared `kp/ki/kd` set with
  a `MotorSettings` table in `lib/Lpf2/src/Lpf2/Local/PortPID.cpp`,
  one entry per `DeviceType`.
- SPEED mode uses the motor's self-reported speed (mode 1, `% of rated`)
  as feedback, eliminating the rated-max-speed scaling mismatch that
  caused oscillation around setpoint.
- POSITION/HOLD share a trapezoidal-decel ramp + pct-domain PD with
  stiction-kick + kinetic-floor friction compensation, so the motor
  clears static friction cleanly without integrator wind-up.
- `startSpeed(0)` now routes to `BrakingStyle::HOLD` instead of running
  an active 0-target speed loop.
- New top-level `Lpf2::Battery` API
  (`lib/Lpf2/include/Lpf2/Battery.hpp`) — single source of truth for
  battery voltage. Defaults to 9000 mV. Supports manual updates via
  `setCurrentVoltage()` or an optional ESP-IDF ADC reader with
  voltage-divider config (`setupAdcDivider` + periodic
  `readBatteryVoltage()`). Percent mapping pluggable; default is linear
  with V_min cutoff.
- `PortPID` applies an over-voltage cap derived from
  `Battery::getCurrentVoltage()` so the motor never sees above its
  nameplate `max_voltage_mv` on an over-spec supply.

### New files

- `lib/Lpf2/include/Lpf2/Battery.hpp`,
  `lib/Lpf2/src/Lpf2/Battery.cpp` — battery API + ADC reader.
- `lib/Lpf2/docs/motor-tuning.md` — per-motor calibration procedure,
  field reference, and battery-integration guide.

### Migration

Battery integration is optional; existing code keeps working with the
default 9000 mV. To enable live tracking:

```cpp
#include "Lpf2/Battery.hpp"

// Manual updates from your own ADC code:
Lpf2::Battery::setCurrentVoltage(read_mv());

// Or use the built-in ADC + divider reader:
Lpf2::Battery::AdcConfig cfg{
    .adc_channel    = ADC_CHANNEL_0,
    .adc_unit       = 1,
    .r_top_ohms     = 10000.0f,
    .r_bottom_ohms  = 4700.0f,
};
Lpf2::Battery::setupAdcDivider(cfg);
// then periodically:
Lpf2::Battery::readBatteryVoltage();
```

## 2.3.0 — 2026-06-23

- `Port` now owns and manages its attached `Device` directly. Call
  `port.init()` once, then `port.update()` in your loop, then
  `port.device()` to get the current device — no more separate
  `DeviceManager`.
- Handed-out `Device*` pointers are now safely invalidated when the
  port swaps devices (e.g. unplug + plug different type). The old
  device is freed and any stale handle stored by the caller is reported
  via a generation counter on the new `DeviceSlot`, rather than
  dangling.

### Breaking changes

Despite the minor version bump, this release changes behavior that
existing callers may rely on:

- `Lpf2::DeviceManager` no longer owns the device. It is now a
  `[[deprecated]]` thin wrapper that forwards `init/update/device` to
  the underlying `Port`. Existing code keeps compiling and working but
  the class will be removed in a future release.
- `DeviceManager::device()` previously returned a raw pointer that
  could dangle after `update()`. It now returns the slot-managed
  pointer — callers that stored the old raw pointer across an
  `update()` should re-fetch via `port.device()`.

```cpp
// before
Lpf2::DeviceManager dm(portA);
dm.init();
dm.update();
auto* dev = dm.device();

// after
portA.init();
portA.update();
auto* dev = portA.device();
```

### Internal

- New `Lpf2::DeviceSlot { Device* ptr; uint32_t gen; }` handle, shared
  between port and device consumers. Underlying device storage is
  `std::unique_ptr<Device>` on the port. `Port::swapDevice()` nulls the
  current ptr, deletes the old device, installs the new one, and bumps
  `gen` so consumers caching the previous generation can detect the
  change.
- `Port::manageDevice()` carries the factory-resolution loop that used
  to live in `DeviceManager::attachViaFactory()`.

## 2.2.0 — 2026-06-16

- Combined-mode support across Hub and Port: combo pairs setup,
  `sendCombinedModeFormat`, `CMD_WRITE` with combined-mode flag and
  pair count, active combo selection logic.
- `Hub::combinedMode` setup API and `Hub::waitPending` for handling
  request timeouts.
- Logging: explicit `log` init in setup paths; safer serial writes;
  mutex hardening in `lpf2_log_printf`.
- Motor: position handling and PID parameters tuned for accuracy;
  speed/position commands deduplicated to avoid redundant updates.
- Bit-pointer handling added for mode/dataset pairs when building
  messages.
- Arduino serial preprocessor guard corrected.

## 2.1.9 — 2026-06-02

- Fix payload index usage in `PortParser` and `HubEmulation`.
- Remove leftover `setPower` from `Virtual::Device`.

## 2.1.8 — 2026-05-10

- Remove unused/leftover method from `Virtual::Device`.

## 2.1.7 — 2026-05-10

- Trivial fixes to data writing path (follow-up to 2.1.6).

## 2.1.6 — 2026-05-10

- `Local::Port` now uses the `Writer` class to send data messages.

## 2.1.5 — 2026-05-10

- Fix binding for value-change callback in `EmulatedPort` and `Port`.

## 2.1.4 — 2026-05-10

- Rename `poll` → `update` across device classes.

## 2.1.3 — 2026-05-10

- Add value-change callback functionality to devices.

## 2.1.2 — 2026-05-09

- Fix encoder-motor factory name.

## 2.1.1 — 2026-05-09

- Split README into focused `docs/` files; update architecture docs.
- Local emulated devices feature merged.
- Example updated to current API.

## 2.1.0 — 2026-05-09

- New `EmulatedPort` class with host-connection check, message
  handling, `discardRxFiFo` / `clearBuf` helpers, device version
  handling.
- Major device-architecture refactor; motor control enhancements;
  built-in PID implementation; updated motor control parameters and
  logging.
- Descriptor library updated: new fields, corrected values, missing
  descriptors added, correct flags.
- More robust UART detection.

## 2.0.13 — 2026-05-06

- Arduino-core compatibility via `LPF2_USE_ARDUINO_SERIAL` flag.

## 2.0.12 — 2026-04-08

- Basic port class exposes its members to `HubEmulation`.

## 2.0.11 — 2026-04-07

- Add default initializers to `Lpf2::Version`.

## 2.0.10 — 2026-04-07

- HubEmulation: add disconnect support; `stop()` deinits NimBLE stack;
  `end` → `stop` rename; non-blocking message loop; null check and
  task cleanup in `msgTask`; consistent member naming across
  `HubEmulation` and `Port`; `m_modes` → `m_modeCount`; built-in
  devices default off; `update` called in message loop.
- Spelling fix: `capabilities` (was misspelled in multiple places).
- Rename functions to more meaningful names.
- Runtime log-level support.
- UART interface: add `read` method; improved serial data handling
  and logging; adjusted message length in port input format handling.
- Remove `Arduino.h` dependency.
- Consistent data types; default initializations.

## 2.0.9 — 2026-03-31

- Rename `writeValue` → `writeResponse`; add 10 ms delay to avoid
  overloading the client.
- README: PlatformIO registry badge.

## 2.0.8 — 2026-03-29

- Fix output command feedback.
- Map raw speed 127 to 0.
- Add missing include.
- Copyright and licensing notices added to source files.
- README: note on local port requirements.

## 2.0.7 — 2026-03-07

- Version bump only (release/packaging).

## 2.0.6 — 2026-03-05

- HubEmulation: non-blocking message callback; refactor to use
  `LPF2_GET_TIME()` for time measurements; consistent device
  descriptor names; improved logging.

## 2.0.5 — 2026-02-22

- Version bump only (release/packaging).

## 2.0.4 — 2026-02-22

- Fix info phase.
- `deviceDataReceived` flag added; updated handling in `Port` classes.
- README clarifications.

## 2.0.3 — 2026-02-21

- Refactor `Port` and `Parser` initialization.

## 2.0.2 — 2026-02-21

- `Local::Port` / `Remote::Port`: Ctrl+C restart feature.
- `HubEmulation` MAC address handling improvements.
- Switch metadata from `library.json` to `library.properties` for
  PlatformIO + Arduino compatibility.

## 2.0.1 — 2026-02-15

- Finish file moves from the library-restructure.
- Add `CODEOWNERS`.
- README disclaimers.

## 2.0.0 — 2026-02-15

- **Breaking:** code moved into namespaces (`Lpf2::…`) — public API
  symbols change, hence the major bump.
- Library structure reorganised; includes updated.

## 1.0.2 — 2026-02-15

- Description update; metadata-only release.

## 1.0.1 — 2026-02-15

- New `EmulatedHub` example; improved advertisement-data logging.
- Fix `onDisconnect`; example sets hub LED green after connect.
- Motor control refactor with new functionality.
- Header rename `.h` → `.hpp`.
- Credit fixes; `.gitignore` updated to exclude `Lpf2*.tar.gz`.
- `library.json` keywords enhanced.

## 1.0.0 — 2026-02-07

- Initial release: first tagged version with examples.
