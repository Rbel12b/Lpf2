# Remote Port (Hub-connected Device)

`Remote::Port` proxies a device attached to a LEGO Hub over BLE.

## Connecting to a hub

```cpp
#include "Lpf2/Hub.hpp"

Lpf2::Hub hub;

void loop()
{
    vTaskDelay(1);

    if (!hub.isConnected() && !hub.isConnecting())
    {
        hub.init();
        vTaskDelay(500);
    }

    if (hub.isConnecting())
    {
        hub.connectHub();
        if (hub.isConnected())
            Serial.println("Connected");
    }

    if (hub.isConnected())
    {
        // use hub ports here
    }
}
```

## Using a port

```cpp
// Get port A from a Technic Hub
auto &portA = *hub.getPort(Lpf2::PortNum(Lpf2::ControlPlusHubPort::A));

portA.update();

if (portA.getDeviceType() == Lpf2::DeviceType::TECHNIC_COLOR_SENSOR)
{
    portA.setMode(0);                        // call once after device connects
    Serial.println(portA.getValue(0, 0));    // color index
}

if (portA.getDeviceType() == Lpf2::DeviceType::TECHNIC_LARGE_LINEAR_MOTOR)
{
    portA.startSpeed(50, 100);
}
```

## Typed device access on a remote port

```cpp
portA.update();

if (auto *dev = portA.device())
{
    if (auto *sensor = static_cast<Lpf2::Devices::TechnicColorSensorControl *>(
            dev->getCapability(Lpf2::Devices::TechnicColorSensor::CAP)))
    {
        Serial.println(sensor->getColorIdx());
    }
}
```

See [device-manager.md](device-manager.md) for the capability API and the
Port-owned device lifetime model.

### Alternative: RTTI / `dynamic_cast`

If your build enables RTTI (`-frtti`), you can skip the capability lookup and
cast directly to a control interface. See
[device-manager.md](device-manager.md#alternative-typed-access-via-rtti) for the
build setup and tradeoffs.

```cpp
portA.update();

if (auto *dev = portA.device())
{
    if (auto *sensor = dynamic_cast<Lpf2::Devices::TechnicColorSensorControl *>(dev))
    {
        Serial.println(sensor->getColorIdx());
    }
    else if (auto *motor = dynamic_cast<Lpf2::Devices::EncoderMotorControl *>(dev))
    {
        motor->startSpeed(50);
    }
    else if (auto *motor = dynamic_cast<Lpf2::Devices::BasicMotorControl *>(dev))
    {
        motor->setSpeed(50);
    }
}
```

Full example: `examples/RemotePortRtti/`. PlatformIO env
`esp32_remote_port_rtti` already sets the required build flags.

## Hub emulation

See `HubEmulation.hpp` and the `EmulatedHub` example. Emulation uses `Virtual::Port` internally.
