# Lpf2 Library for ESP32

[![PlatformIO Registry](https://badges.registry.platformio.org/packages/rbel12b/library/Lpf2.svg)](https://registry.platformio.org/libraries/rbel12b/Lpf2)

LPF2 (LEGO Power Functions 2 / PoweredUp) communication library for ESP32. Supports:

- Driving physical LPF2 devices over UART (`Local::Port`)
- Reading sensors and controlling motors attached to LEGO Hubs over BLE (`Remote::Port`)
- Emulating a LEGO Hub (`HubEmulation`)
- **Presenting the ESP32 as a custom LPF2 device** to any host hub or controller (`Local::EmulatedPort`)

Download from the PlatformIO registry: [rbel12b/Lpf2](https://registry.platformio.org/libraries/rbel12b/Lpf2)

---

## Documentation

| Doc | Contents |
| --- | --- |
| [Architecture](docs/architecture.md) | Class hierarchy, port types, data flow diagrams |
| [Getting Started](docs/getting-started.md) | Installation, build flags, required startup calls |
| [Local Port (Master)](docs/local-port.md) | Drive a physical LPF2 device over UART |
| [Emulated Port (Slave)](docs/emulated-port.md) | Present the ESP32 as a custom LPF2 device |
| [Remote Port](docs/remote-port.md) | Control devices through a LEGO Hub over BLE |
| [Device Manager & Capabilities](docs/device-manager.md) | Auto device lifecycle, capability interface, RTTI alternative |
| [Custom Device](docs/custom-device.md) | Add a new typed device + factory |
| [Logging](docs/logging.md) | Compile-time and runtime log level, output destination |

---

## Credits

- [Legoino](https://github.com/corneliusmunz/legoino) by Cornelius Munz — reference and base for LWP hub control/emulation
- [LEGO Wireless Protocol documentation](https://lego.github.io/lego-ble-wireless-protocol-docs)
- [LEGO PoweredUp UART Protocol](https://github.com/pybricks/technical-info/blob/master/uart-protocol.md) by the pybricks team
- [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) by h2zero (Ryan Powell)

## Disclaimers

LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this project.

## License

GNU Affero General Public License v3.0 (AGPL-3.0). See [LICENSE](./LICENSE).
