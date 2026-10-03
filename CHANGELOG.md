# Changelog

## 2.8.1 — 2026-10-03

### Thread-safe value-change callback (`Port`)

`setValueChangeCallback` and `fireValueChangeCallback` now hold
`m_callbackMutex` around the callback pointer access, preventing a
data race when the callback is replaced from one thread while the port
fires it from another.

- `m_callbackMutex` is a `Lpf2::Utils::Mutex` (FreeRTOS `SemaphoreHandle_t`
  or no-op on bare-metal); initialised with `LPF2_MUTEX_CREATE()` under
  `LPF2_USE_FREERTOS`.
- Null-check moved inside the lock in `fireValueChangeCallback` so the
  guard and the call are atomic.
- `setValueChangeCallback` moved out of the header into `Port.cpp` to
  keep the lock scope out of inline code.

## 2.8.0 — 2026-10-01

### `Local::Port::getIO()` — expose IO reference

`Local::Port` gains a public accessor:

```cpp
IO& getIO();
```

Returns the `IO` object that owns the port's UART and PWM. Needed by
bindings that borrow the UART for a slave (`EmulatedPort`) after
disabling the port's own scanning.

### UART protocol fixes

- **Master break-condition detection:** `PortAnalog` now tries UART
  fast-init first when `ch0 ≥ 3V + ch1 ≈ 0V` (break condition).
  Falls back to `TRAIN_MOTOR` only after the 300 ms fast-init window
  expires. Analog cycles reduced to 5 samples (25 ms) for faster
  response.
- **Universal 300 ms UART timeout:** `STATUS_SPEED_CHANGE` now falls
  back to analog scan after 300 ms regardless of how it was entered,
  not only from the break-condition path.
- **Correct ACK after speed change:** `STATUS_SPEED` now sends ACK
  (0x04) instead of NACK (0x02) when confirming the host's speed
  change.
- **Early ACK from descriptor registry:** When a matching
  `DeviceDescriptor` is found in `CMD_TYPE`, the hub skips the full
  INFO sequence and jumps straight to `STATUS_ACK_SENDING`, cutting
  enumeration time for known device types to near-zero.

### `EmulatedPort` protocol fixes

- `reset()` now starts in `DETECTING_HOST` (holds TX low) instead of
  jumping directly to `SENDING_INFO`, so a real LPF2 hub can perform
  the `CMD_SPEED` handshake.
- `CMD_SPEED` handler resets all info-sequence counters
  (`m_infoNum`, `m_infoSubNum`, `m_infoState`) so the device always
  starts from `CMD_TYPE` after a speed negotiation.
- Mode info is now sent in descending order (N-1 … 0) as required by
  the LPF2 spec.
- `INFO_FORMAT` is now the final per-mode message; `INFO_MODE_COMBOS`
  follows FORMAT on mode 0 only.

### `EmulatedPort` slow-enumeration helper

- Default detection baud is **115200** (was always the runtime default;
  the doc note "starts at 2400" was wrong).
- `WAITING_FOR_HOST` no longer times out — waits indefinitely for
  `CMD_SPEED`. Use `setSlowEnumeration(true)` for EV3 hosts.
- New `setSlowEnumeration(bool slow)` — call before `init()` to go
  directly to 2400 baud EV3 mode without waiting.
- New `isSlowEnumeration() const`.

### `EmulatedPort`: fix premature `SENDING_DATA` on baud-change ACK

- `parseMessage` no longer transitions to `SENDING_DATA` when a
  `BYTE_ACK` arrives during `SENDING_INFO`. The master's ACK confirming
  the `CMD_SPEED` baud negotiation was being mishandled, causing the
  INFO exchange to abort early and triggering a ~1 s re-enumeration
  loop when a port expander device was attached.

## 2.7.0 — 2026-09-06

### Optional BLE stack ownership in `HubEmulation`

`HubEmulation` now accepts an optional `ownsBleStack` constructor parameter:

```cpp
HubEmulation(std::string hubName, HubType hubType, bool ownsBleStack = true);
```

Pass `false` when the NimBLE stack is managed externally (e.g. shared with a
MicroPython `bluetooth` module). When `ownsBleStack` is `false`, `start()` skips
NimBLE init and `stop()` skips `nimble_port_stop` / `nimble_port_deinit`, so the
two users coexist without double-init or early teardown.

The hub device name is now set in the BLE advertisement at init time rather than
being left to the default.

### Weak BLE chain hooks

Three `extern "C" __attribute__((weak))` hooks let an external BLE module observe
NimBLE server events without patching `HubEmulation`:

- `lpf2_chain_on_connect(conn_handle, addr_type, addr[6])`
- `lpf2_chain_on_disconnect(conn_handle, addr_type, addr[6])`
- `lpf2_chain_on_mtu_change(conn_handle, mtu)`

If absent the linker resolves them as no-ops.

### Port disable API

`Port` now has a pause/resume mechanism that stops `update()` polling without
destroying the device wrapper:

- `port.disable(bool disable = true)` — pause or resume; fires `_onDisable(bool)` only on transition.
- `port.isDisabled() const` — query current state.
- `Port::_onDisable(bool)` — virtual hook for subclasses to release / reacquire transport resources (e.g. UART deinit/init).

### EV3 motor support via `forceDeviceType`

`Local::Port` gains two new methods for devices that skip the LPF2 handshake
(EV3 motors, bare PWM outputs):

- `port.forceDeviceType(DeviceType type)` — disables UART detection and fixes the
  reported device type. If a descriptor is registered for `type`, mode/combo data
  is populated via `setFromDesc()` so motor commands (`setPower`, `startPower`,
  etc.) work immediately.
- `port.enable()` — convenience wrapper for `disable(false)`; restores normal UART
  detection.

New `DeviceType` constants for EV3 motors added to `LWPConst.hpp`.

### BLE characteristic discovery cleanup

SPIKE Prime hub handling removed from the BLE characteristic discovery path.
Discovery now targets Control+/Technic hub characteristics only.

### Logging output redirect

`lpf2_log_set_vprintf(vprintf_like_fn)` added to `include/Lpf2/log/log.h`.
Pass a custom `vprintf`-compatible function to redirect log output (e.g. into
MicroPython's USB CDC path). Pass `nullptr` to restore the default `vprintf`.

---

## 2.6.1 — 2026-07-13

### New device: `Lpf2::Devices::ColorDistanceSensor`

Full driver for the LEGO Color & Distance Sensor (device type 37):

- `getColorIdx()` — detected color as `ColorIDX`.
- `getDistance()` — proximity reading (0.0–1.0).
- `getReflectedLight()` — reflected light percentage.
- `getAmbientLight()` — ambient light percentage.
- `getRgb(r, g, b)` — raw RGB channels.
- `setIrTx(value)` — write IR TX pattern.
- `setLedColor(color)` — set the sensor's built-in LED.
- `setMode(modeNum, delta)` — switch single mode; returns the mode number set.
- Mode constants: `MODE_COLOR`, `MODE_DIST`, `MODE_REFLT`, `MODE_AMBI`,
  `MODE_LED`, `MODE_RGB`, `MODE_IR`.

### Extended `Lpf2::Devices::ColorSensor`

- `getAmbientLight()` — ambient light reading.
- `getReflectivity()` — reflected light percentage.
- `getRGB(r, g, b)` — raw RGB channels.
- `setMode(modeNum, delta)` — single-mode selection.

### `setMode` / `setModeCombo` return values

`setMode()` and `setModeCombo()` on both `ColorDistanceSensor` and `ColorSensor`
now return the mode number that was applied (`int`), matching the pattern
established by `TechnicColorSensor`.

### Remote port timing fix

`Hub::writeValue` now forces an immediate flush on remote ports so commands
reach the connected hub without waiting for the next scheduled update cycle.

### BLE null-pointer safety

`BLEAddress` and `BLERemoteCharacteristic` member pointers in `Hub` are now
initialised to `nullptr`, preventing potential use-before-init crashes during
early disconnect handling.

---

## 2.6.0 — 2026-07-09

Added new devices under `Lpf2::Devices` namespace:

- `Lpf2::Devices::HubLed` — a device that controls the hub's LED, allowing users to set the LED color.
- `Lpf2::Devices::HubAccelerometer` — a device that provides access to the hub's accelerometer, allowing users to read acceleration data.
- `Lpf2::Devices::HubGyroscope` — a device that provides access to the hub's gyroscope, allowing users to read angular velocity data.

Updated `Lpf2::Devices::TechnicColorSensor` with new methods:

- `getReflectivity()` — get reflected light percentage (mode 1 = REFLT, 0-100 PCT).
- `getRGB(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &i)` — get raw RGB channels and intensity (mode 5 = RGB I).
- `getHSV(uint16_t &h, uint16_t &s, uint16_t &v)` — get HSV readings (mode 6 = HSV; H 0-360, S 0-100, V 0-360).
- `setLight(uint8_t l1, uint8_t l2, uint8_t l3)` — set the on-board light channels (mode 3 = LIGHT; 0-100 PCT each).

## 2.5.3 — 2026-07-06

Fixed the `CMD_EXT_MODE` bug in `Local::PortParser`

## 2.5.2 — 2026-07-06

Fixed the `CMD_EXT_MODE` bug in `Local::EmulatedPort` and
`Local::PortWriter` where the flag was not being reset after sending the
extended mode message, causing the next mode to always be treated as
extended.

## 2.5.1 — 2026-07-06

Fix mode selection: Do not send `CMD_EXT_MODE`, just send the mode number
as is. The `CMD_EXT_MODE` is only used in data messages.

## 2.5.0 — 2026-06-28

Added a new `getSpeed()` and `getAbsPosition()` method to the
`EncoderMotorControl` interface, allowing users to retrieve the speed and
position measured by the encoder.

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
