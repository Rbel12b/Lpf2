# Devices on a Port

> Since **v2.3.0**, `Port` owns and manages its connected `Device` directly.
> The previous `Lpf2::DeviceManager` class is now a deprecated thin wrapper
> that forwards to the underlying port.

A `Port` automatically constructs the right typed `Device` when one is
connected, using the registered `DeviceFactory` instances.

## Setup

```cpp
Lpf2::DeviceRegistry::registerDefault(); // once at startup
port.init();                              // once per port, before update() loop
```

## Basic usage

```cpp
void loop()
{
    port.update(); // polls transport; updates the attached device if present

    if (auto *dev = port.device()) // attaches via factory on first call after connect
    {
        // a device is connected and constructed
    }
}
```

`port.update()` polls the underlying transport and forwards `update()` to
the currently attached device. `port.device()` lazily attaches a typed
device the first time it is called after a connect — subsequent calls
return the same instance until the device is unplugged or replaced.

## Device lifetime & invalidation

The port owns the device via `std::unique_ptr<Device>`. Consumers receive
a non-owning pointer. To make stale pointers safe, the port holds a
`std::shared_ptr<DeviceSlot>` where:

```cpp
struct DeviceSlot {
    Device   *ptr;   // current device, or nullptr after a swap
    uint32_t  gen;   // bumped on every swap
};
```

When the port swaps the device (disconnect or device-type change), it:

1. Nulls `slot->ptr` so any cached handle sees `nullptr`.
2. Deletes the old device.
3. Installs the new device and increments `slot->gen`.

Wrappers that cached the previous `gen` can detect the change and refuse
to dereference the (now-deleted) old device.

## Capabilities

Devices expose typed interfaces via capability IDs. IDs are compile-time
FNV-1a hashes — no runtime table required.

```cpp
if (auto *motor = static_cast<Lpf2::Devices::BasicMotorControl *>(
        port.device()->getCapability(Lpf2::Devices::BasicMotor::CAP)))
{
    motor->setSpeed(50);
}

if (auto *sensor = static_cast<Lpf2::Devices::TechnicColorSensorControl *>(
        port.device()->getCapability(Lpf2::Devices::TechnicColorSensor::CAP)))
{
    Serial.println(sensor->getColorIdx());
}
```

`getCapability()` returns `nullptr` if the connected device does not
implement that capability.

## Built-in capabilities

| Capability class | CAP constant | Devices |
| --- | --- | --- |
| `BasicMotorControl` | `BasicMotor::CAP` | All motors |
| `EncoderMotorControl` | `EncoderMotor::CAP` | Technic motors with encoder |
| `TechnicColorSensorControl` | `TechnicColorSensor::CAP` | Technic color sensor |
| `DistanceSensorControl` | `DistanceSensor::CAP` | Technic distance sensor |

`EncoderMotor` also implements `BasicMotorControl` — all motors share that
capability.

## Custom device factory

```cpp
class MyFactory : public Lpf2::DeviceFactory
{
public:
    bool matches(const Lpf2::Port &port) const override
    {
        return port.getDeviceType() == Lpf2::DeviceType::MY_TYPE;
    }

    Lpf2::PortDevice *create(Lpf2::Port &port) const override
    {
        return new MyDevice(port);
    }

    const char *name() const override { return "My Device Factory"; }
};

namespace { MyFactory factory; }

// Call once at startup, before any port.device() call.
Lpf2::DeviceRegistry::instance().registerFactory(&factory);
```

See [custom-device.md](custom-device.md) for a full example with capability
interfaces.

## Legacy `DeviceManager`

The class still exists in `Lpf2/DeviceManager.hpp` for backwards
compatibility but is marked `[[deprecated]]`. Migration is mechanical:

```cpp
// before
Lpf2::DeviceManager dm(portA);
dm.init();
dm.update();
auto *dev = dm.device();

// after
portA.init();
portA.update();
auto *dev = portA.device();
```
