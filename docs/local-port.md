# Local Port (Master)

`Local::Port` drives a physical LPF2 device connected over UART. The ESP32 is the master; the LEGO device (motor, sensor) is the slave.

## Hardware requirements

- H-bridge motor driver for motor ports
- 47 kΩ pull-up resistors on the ID1 and ID2 pins

## IO class

You must provide an IO implementation (from an example or your own):

```cpp
#include "Lpf2/Local/IO/UART.hpp"

// Implement Lpf2::Local::IO for your board — see examples/LocalPort/device.h
class Esp32IO : public Lpf2::Local::IO { ... };
```

## Basic usage

```cpp
Esp32IO portA_IO(1);               // UART1
Lpf2::Local::Port portA(&portA_IO);

void setup()
{
    Lpf2::DeviceDescRegistry::registerDefault();
    Lpf2::DeviceRegistry::registerDefault();

    portA_IO.init(ID1_PIN, ID2_PIN, PWM1_PIN, PWM2_PIN, MCPWM_UNIT_0, MCPWM_TIMER_0, 1000);
    portA.init();
}

void loop()
{
    vTaskDelay(1);
    portA.update();

    if (portA.getDeviceType() == Lpf2::DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR)
        portA.startPower(80);

    if (portA.getDeviceType() == Lpf2::DeviceType::TECHNIC_COLOR_SENSOR)
        Serial.println(portA.getValue(0, 0));
}
```

## Typed device access

Since v2.3.0, the port owns its connected device. Call `portA.device()`
to get the typed `Device` instance — the right type is constructed
lazily via the registered factories:

```cpp
portA.update();

if (auto *dev = portA.device())
{
    if (auto *motor = static_cast<Lpf2::Devices::BasicMotorControl *>(
            dev->getCapability(Lpf2::Devices::BasicMotor::CAP)))
    {
        motor->setSpeed(50);
    }
}
```

See [device-manager.md](device-manager.md) for capability details and
the device-lifetime model.

## Port API

| Method | Description |
| --- | --- |
| `startPower(pw)` | Raw power −100..100 |
| `startSpeed(speed, maxPower, profile)` | Speed with acc/dec profile |
| `startSpeedForTime(ms, speed, maxPower, end, profile)` | Timed move |
| `startSpeedForDegrees(deg, speed, maxPower, end, profile)` | Degree-limited move |
| `gotoAbsPosition(pos, speed, maxPower, end, profile)` | Absolute encoder target |
| `presetEncoder(pos)` | Zero/preset encoder |
| `setMode(mode)` | Select active mode |
| `setModeCombo(idx)` | Select mode combination |
| `getValue(modeNum, dataSet)` | Read last received value |
| `getDeviceType()` | Connected device type |
