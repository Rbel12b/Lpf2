# Motor PID tuning

Per-motor calibration for the encoder-motor PID in
`lib/Lpf2/src/Lpf2/Local/PortPID.cpp`. Every supported motor type has
its own entry in `SETTINGS_TABLE` with the constants listed below.

## PID architecture

Two control modes share one tick (`Port::updateMotorPID()`):

- **SPEED** — pct-domain PI with feed-forward. Output:
  `pwr = m_pidSpeed + speed_ksp * err_pct + speed_ksi * integral_pct_s`
  where `err_pct = m_pidSpeed - reported_pct` and `reported_pct` is the
  motor's mode-1 self-reported speed (combo 0).
- **POSITION / HOLD** — pct-domain PD on position error + a feed-forward
  derived from the trapezoidal target speed:
  `pwr = setpoint_pct + pos_kp * err_deg + pos_kd * speed_err_pct`
  (`pos_ki` is available but defaults to zero — see §"KI use" below).

After PID, two friction helpers run unconditionally:

- **Stiction kick** — when `|err_for_sign| > 0`, `|obs_speed| < 0.1 deg/s`,
  and `|pwr| < breakaway_pct`, output is forced to
  `breakaway_pct * sign(err)`. Bypasses static friction without waiting
  for an integrator.
- **Kinetic floor** — once moving, if `0 < |pwr| < kinetic_floor_pct`,
  output is forced to `kinetic_floor_pct * sign(pwr)`. Avoids the
  dead-band band where PWM is too low to spin.

Finally an **over-voltage cap** (driven by `Lpf2::Battery`) limits PWM
so the motor never sees above `max_voltage_mv` even on an over-spec
battery (see [Battery voltage role](#battery-voltage-role)).

## `MotorSettings` field reference

| Field | Units | Role |
| --- | --- | --- |
| `rated_max_speed` | deg/s | Informational; used only to convert ramp speed mdeg/s ↔ pct |
| `max_voltage_mv` | mV | Motor nameplate max. Drives over-voltage cap |
| `speed_ksp` | pct/pct_err | SPEED proportional gain |
| `speed_ksi` | pct/(pct_err·s) | SPEED integral gain |
| `speed_int_clamp` | pct·s | Anti-windup limit on SPEED integral |
| `speed_deadband_pct` | pct | SPEED err below this → treated as 0 |
| `pos_kp` | pct/deg | POSITION/HOLD proportional gain |
| `pos_ki` | pct/(deg·s) | POSITION/HOLD integral gain (often 0) |
| `pos_kd` | pct/pct_err | POSITION/HOLD derivative on speed error |
| `pos_int_clamp` | deg·s | POSITION integral clamp |
| `pos_deadband_deg` | deg | Position error below this → treated as 0 |
| `pos_decel_mdps2` | mdeg/s² | Trapezoidal deceleration cap |
| `breakaway_pct` | pct | Stiction kick magnitude |
| `kinetic_floor_pct` | pct | Min sustained drive while moving |

## What you'll need

- Motor under test, attached to a Local port.
- A way to observe behaviour: serial monitor + `LPF2_LOG_V` already prints
  `Pos/Rel/ObsA/ObsV` each tick when log level allows it. For raw checks,
  `port.getValue(1, 0)` is the motor's reported speed in pct.
- A known-good power source — ideally a bench supply or a freshly-charged
  pack. Voltage sag invalidates tuning (see Battery section).

## Step-by-step

Run each step per motor type. Update the corresponding `SETTINGS_TABLE`
entry in `lib/Lpf2/src/Lpf2/Local/PortPID.cpp` after each measurement
and rebuild.

### 1. Friction thresholds (`breakaway_pct`, `kinetic_floor_pct`)

```python
motor.startPower(p)
```

- Walk `p` up from 1: the smallest `p` where the motor *starts* turning
  on its own is the **breakaway**. Round up.
- After it spins, drop `p` step by step: the smallest `p` where it keeps
  spinning is the **kinetic floor**.
- Typical PUP: breakaway 20–30 %, kinetic 12–20 %.

### 2. SPEED loop (`speed_ksp`, `speed_ksi`)

```python
motor.startSpeed(50, max_power=100)
```

- Watch `getValue(1, 0)`. Raise `speed_ksp` until you see steady
  oscillation around 50, then back off ~30 %.
- Raise `speed_ksi` until steady-state error falls below 2 % within ~1 s.
  Stop before overshoot reappears.
- Validate with step input: `30 → 80 → -50 → 0`.

### 3. POSITION loop (`pos_kp`, `pos_kd`)

```python
motor.gotoAbsPosition(0, speed=50, max_power=100, end_state=HOLD)
# Manually rotate the motor ~30°, then release.
```

- Raise `pos_kp` until the motor returns within `pos_deadband_deg` with
  no oscillation. Rule of thumb: pick `pos_kp` so that
  `pos_kp * pos_deadband_deg ≥ breakaway_pct + 2` (an error just outside
  the deadband instantly clears static friction).
- Raise `pos_kd` until overshoot vanishes. Too much `pos_kd` makes the
  motor feel sluggish — back off.

### 4. Trapezoidal decel (`pos_decel_mdps2`)

```python
motor.gotoAbsPosition(720, speed=100, max_power=100, end_state=HOLD)
motor.gotoAbsPosition(0,   speed=100, max_power=100, end_state=HOLD)
```

- Overshoots final → lower `pos_decel_mdps2`.
- Brakes too early (slow finish) → raise it.
- Common range: 1.0e6 – 5.0e6 (1000–5000 deg/s²).

### 5. `pos_deadband_deg`

Smallest residual error you tolerate at rest. 0.3 ° is fine for most
PUP motors; raise to 1 ° if you see chatter at the deadband edge.

### KI use

`pos_ki` is wired but defaults to `0`. Wind-up around static friction is
the usual cause of HOLD oscillation. If your motor needs an integrator
to settle exactly on target (e.g. heavy gear train with backlash), add
a small value (≤ `pos_kp / 50`) and verify the stiction kick still
clears it cleanly.

## Adding a new motor type

1. Add the new `DeviceType` enum value to `lib/Lpf2/include/Lpf2/LWPConst.hpp`
   if it isn't already there.
2. Register the device factory so `EncoderFactory::matches()` accepts
   it — see `lib/Lpf2/src/Lpf2/Devices/EncoderMotor.cpp:41`.
3. Append an entry to `SETTINGS_TABLE` in
   `lib/Lpf2/src/Lpf2/Local/PortPID.cpp` using designated initializers,
   mirroring the layout of an existing entry. Run §1–§5 to fill in the
   numbers.

## Battery voltage role

Gains tuned on a fresh battery drift as the pack sags — the same PWM
percentage now drives less actual voltage, so the motor reaches a lower
top speed and PID gains under-shoot. The PortPID over-voltage cap also
needs a current voltage reading to avoid over-volting the motor when
the supply is above `max_voltage_mv`.

`Lpf2::Battery` (`lib/Lpf2/include/Lpf2/Battery.hpp`) is the single
source of truth. Defaults: `max = 9000 mV`, `min = 6000 mV`,
`current = 9000 mV`.

### Manual update

```cpp
#include "Lpf2/Battery.hpp"

void loop_every_second()
{
    uint16_t mv = read_my_adc_or_smbus();
    Lpf2::Battery::setCurrentVoltage(mv);
}
```

### ADC + voltage divider

The lib can read the battery itself via ESP-IDF ADC (with curve-fitting
calibration) given a resistor divider. Call once at startup:

```cpp
Lpf2::Battery::AdcConfig cfg{
    .adc_channel    = ADC_CHANNEL_0,   // adc_channel_t value
    .adc_unit       = 1,                // ADC_UNIT_1
    .r_top_ohms     = 10000.0f,         // V_batt -> tap
    .r_bottom_ohms  = 4700.0f,          // tap -> GND
    .vref_mv        = 3300,
    .samples        = 8,
};
Lpf2::Battery::setupAdcDivider(cfg);
```

Then call `Lpf2::Battery::readBatteryVoltage()` from your loop at the
cadence you want updates (1 Hz is plenty). It averages `samples` raw
reads, calibrates them, scales by `(r_top + r_bottom) / r_bottom`, and
stores the result as the current voltage.

### Percent

`Lpf2::Battery::getPercent()` returns 0–100 using a linear mapping with
`V_min` cutoff by default. Override with your own curve:

```cpp
uint8_t my_curve(uint16_t mV, uint16_t vmin, uint16_t vmax) { /* … */ }
Lpf2::Battery::setPercentFunc(my_curve);
```

### Reporting to the LWP hub property

Battery percent surfaces over BLE through
`HubEmulation::setBatteryLevel(uint8_t)`. The Battery API stays
HAL-free — wire them yourself from your application loop:

```cpp
hub.setBatteryLevel(Lpf2::Battery::getPercent());
```
