# Virtual Port (Hub Emulation)

`Virtual::Port` is the port type used **inside** a `HubEmulation` context. It has no UART or BLE of its own — instead it holds a `Virtual::Device` descriptor and lets the emulated hub answer LWP port queries (mode info, output commands, sensor values) as if a real LEGO device were attached.

Use this when you want a hub emulated by `HubEmulation` to expose a device on one of its ports.

## When to use which port

| Port type | Purpose |
| --- | --- |
| `Local::Port` | ESP32 is master, drives a real LPF2 device over UART |
| `Local::EmulatedPort` | ESP32 is slave, presents itself as a LPF2 device over UART |
| `Remote::Port` | ESP32 talks to a device attached to a real LEGO Hub over BLE |
| `Virtual::Port` | ESP32 emulates a hub, and the port is a slot on that emulated hub |

`Virtual::Port` is only meaningful when passed to `HubEmulation::attachPort`. It is what the emulated hub uses to answer clients (e.g. a phone app talking to the emulated hub over BLE).

## How it works

`Virtual::Port` derives from `Lpf2::Port`. It overrides every command/query the base port defines and forwards it to an attached `Virtual::Device`:

- `writeData`, `setMode`, `setModeCombo` → forwarded to the device
- Motor commands (`startPower`, `startSpeed`, `startSpeedForTime`, `gotoAbsPosition`, ...) → forwarded to the device
- `isDeviceConnected()` returns `true` when a device is attached
- When the device raises a value change (via its `ValueChangeCallback`), the port refreshes its cached mode data and fires its own value-change callback, which `HubEmulation` uses to push `PORT_VALUE_SINGLE` / combined updates to subscribed clients

On `attachDevice(Device*)` the port copies the device's static descriptor fields into itself:

```text
m_deviceType, m_modeData, m_modeCombos,
m_capabilities, m_inModesMask, m_outModesMask,
m_fwVersion, m_hwVersion
```

These are what the emulated hub reports in `PortInformation` / `PortModeInformation` responses.

## Virtual::Device

`Virtual::Device` is a pure interface deriving from `Lpf2::Device`. A subclass must provide the descriptor (`getModes`, `getModeCount`, `getInputModes`, `getOutputModes`, `getCapabilities`, `getModeCombos`, `getFwVersion`, `getHwVersion`, `getDeviceType`) plus implementations for the motor / sensor entry points (`startPower`, `startSpeed`, ...). Sensor data is published by mutating the mode's `rawData` and calling the stored `ValueChangeCallback(modeNum)`.

### Virtual::GenericDevice

`Virtual::GenericDevice` is a ready-made `Virtual::Device` built from a `DeviceDescriptor`. It stores the descriptor, exposes it through the getters, and provides no-op motor implementations that just log the call. Two extension hooks let you plug in real behavior without subclassing:

- `setModeData(modeNum, bytes)` — replace a mode's raw payload and automatically notify the port
- `setWriteDataCallback(cb)` — receive host writes (`writeData`) as a `std::function`
- `setUserData(ptr)` — arbitrary pointer passed back to the write callback

This is the device type `HubEmulation::setUseBuiltInDevices(true)` uses for every built-in port on the emulated hub.

## Usage — custom port on an emulated hub

```cpp
#include "Lpf2/HubEmulation.hpp"
#include "Lpf2/Virtual/Port.hpp"
#include "Lpf2/Virtual/Device.hpp"
#include "Lpf2/DeviceDescLib.hpp"

Lpf2::HubEmulation hub("Technic Hub", Lpf2::HubType::CONTROL_PLUS_HUB);

Lpf2::Virtual::Port    tempPort;
Lpf2::Virtual::GenericDevice tempDevice(Lpf2::DeviceDescriptors::TEMPERATURE_SENSOR);

void setup()
{
    Lpf2::DeviceDescRegistry::registerDefault();

    tempDevice.setWriteDataCallback(
        [](uint8_t mode, const std::vector<uint8_t> &data, void *ud) {
            // host wrote something (e.g. mode-specific config)
            return 0;
        });

    tempPort.attachDevice(&tempDevice);
    hub.attachPort(Lpf2::PortNum(Lpf2::ControlPlusHubPort::TEMP), &tempPort);

    hub.start();
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(100));

    int16_t degC = readTemperature();
    tempDevice.setModeData(0, {
        (uint8_t)(degC & 0xFF),
        (uint8_t)((degC >> 8) & 0xFF),
    });
    // setModeData() fires the value-change callback → HubEmulation streams
    // the new value to any subscribed BLE client automatically.
}
```

Lifetime rule (from `HubEmulation::attachPort`): the port object must outlive the `HubEmulation` instance. The same applies to the `Virtual::Device` attached to it.

## Usage — custom Virtual::Device subclass

Subclass `Virtual::Device` (not `GenericDevice`) when you need custom motor behavior — e.g. an emulated encoder motor that maintains its own position and honors `startSpeedForDegrees` / `gotoAbsPosition`. Override every pure-virtual method; publish sensor updates by mutating your mode table and calling the stored `m_valueChangeCallback(modeNum)`.

```cpp
class VirtualEncoderMotor : public Lpf2::Virtual::Device
{
public:
    // descriptor getters ...
    void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile) override
    {
        m_targetSpeed = speed;
    }
    void presetEncoder(int32_t pos) override
    {
        m_position = pos;
        publishPosition();
    }

private:
    void publishPosition()
    {
        m_modes[0].rawData = encodeInt32(m_position);
        if (m_valueChangeCallback) m_valueChangeCallback(0);
    }
    // ...
};
```

## Built-in ports

`HubEmulation::setUseBuiltInDevices(true)` calls `initBuiltInPorts()` + `initBuiltInDevices()`, which for `CONTROL_PLUS_HUB` create a `Virtual::Port` + `Virtual::GenericDevice` pair on each internal port (accelerometer, gyro, LED, current, voltage, temperature, tilt, gesture) using descriptors from `DeviceDescriptors`. Those ports and devices are owned by `HubEmulation` and destroyed with it.

For your own ports, you own the lifetime; use `attachPort` before `hub.start()`.

## Notes

- `Virtual::Port::_update()` is a no-op — the port has nothing to poll. Data flow is push-driven from the device via `ValueChangeCallback`.
- Detaching (`attachDevice(nullptr)` or `detachDevice()`) clears the cached descriptor and reports `UNKNOWNDEVICE`; the emulated hub will emit a detach notification on next tick.
- Motor commands with no device attached are silently dropped; `setMode` / `setModeCombo` return `1` to signal no device.
