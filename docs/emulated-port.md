# Emulated Port (Slave / Custom Device)

`Local::EmulatedPort` is the slave counterpart to `Local::Port`. The ESP32 presents itself as a LPF2 device to a host (a LEGO Hub or another ESP32 running `Local::Port`).

Use this to build custom sensors, actuators, or any peripheral that speaks the LPF2 UART protocol.

## How it works

During `init()`, `EmulatedPort` goes through the LPF2 handshake state machine:

```text
DETECTING_HOST → WAITING_FOR_HOST → HOST_DETECTED
    → SENDING_INFO → WAITING_FOR_ACK → SENDING_DATA
```

It sends the mode table from the attached `Virtual::Device`, negotiates baud rate, acknowledges the host, then enters the data loop.

## Virtual::Device

Implement `Lpf2::Virtual::Device` to define what your custom device advertises:

```cpp
#include "Lpf2/Virtual/Device.hpp"

class MyDevice : public Lpf2::Virtual::Device
{
public:
    // Return mode descriptors — the host uses these for data formatting
    const std::vector<Lpf2::Mode> &getModes() const override { return m_modes; }

    // Called when the host writes data to this device
    int writeData(uint8_t mode, const std::vector<uint8_t> &data) override { ... }

    DeviceType getDeviceType() const override
    {
        return Lpf2::DeviceType::MY_CUSTOM_TYPE;
    }

    // An example sensor
    void setSensorValue(uint8_t val)
    {
        m_mode[0].rawData.resize(1);
        m_mode[0].rawData[0] = val;
    }

private:
    std::vector<Lpf2::Mode> m_modes = { ... };
};
```

## Usage

```cpp
#include "Lpf2/Local/EmulatedPort.hpp"

Lpf2::Local::EmulatedPort port(uart); // uart is derived from Lpf2::Local::Uart
// an implementation is available in examples/LocalPort/device.hpp

MyDevice device;

void setup()
{
    port.attachDevice(device);
    port.init();
}

void loop()
{
    vTaskDelay(1);
    port.update();

    if (port.isHostConnected())
    {
        // device.getModes() is called automatically by port.update()
        // if the mode data is modified the library will send 
        // it automatically to the host
        //
        // device.setSensorValue(x);
    }
}
```

## State machine

| State | Description |
| --- | --- |
| `DETECTING_HOST` | Listening for host sync signal |
| `WAITING_FOR_HOST` | Waiting for host to assert line |
| `HOST_DETECTED` | Host confirmed, baud negotiation starts |
| `SENDING_INFO` | Transmitting mode descriptors to host |
| `WAITING_FOR_ACK` | Waiting for host ACK/NACK |
| `SENDING_DATA` | Normal operation — sending sensor data, receiving commands |

## Notes

- `isHostConnected()` returns `true` only in `SENDING_DATA` state.
- Calling `detachDevice()` resets the state machine.
- Default baud is **115200** (LPF2 fast path). The port holds TX low and waits
  indefinitely for `CMD_SPEED`. Call `setSlowEnumeration(true)` before `init()`
  to use 2400 baud EV3 mode directly without waiting.

## Slow-enumeration mode

For EV3 hosts (or any host that expects 2400 baud from the start), call
`setSlowEnumeration(true)` before `init()`:

```cpp
port.setSlowEnumeration(true); // 2400 baud, no CMD_SPEED wait
port.attachDevice(device);
port.init();
```

`isSlowEnumeration()` returns the current setting.
