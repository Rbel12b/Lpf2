# Changelog

## 2.3.0

### Highlights

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

### Migration

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
