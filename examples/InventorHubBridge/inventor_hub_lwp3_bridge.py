"""LWP3-over-SPIKE bridge for a LEGO MINDSTORMS Robot Inventor / SPIKE Legacy hub.

Runs ON the Inventor hub (NOT on the ESP32 running Lpf2-micropython).

Its purpose:
1. Advertise the LWP3 service UUID so external LWP3 clients discover the hub.
2. Intercept LWP3 frames arriving through the built-in SPIKE serial channel
   (characteristic 9ef58b69-e191-4daf-89d6-9e115258e627) and dispatch them to
   the hub's local port/device API.
3. Emit LWP3 responses back through the same channel so the client sees them
   as notifications.

The C++ side of this project already accepts the SPIKE characteristic as a
transparent LWP3 transport when it recognises the peer as an Inventor hub
(manufacturer data byte3 == 0x51).

Message-type coverage mirrors HubEmulation.cpp:
    HUB_PROPERTIES           (0x01)  full: get/set/enable/disable/reset
    HUB_ACTIONS              (0x02)  disconnect / power off
    HUB_ALERTS               (0x03)  enable/disable/request
    HUB_ATTACHED_IO          (0x04)  emitted on connect + on hot-plug
    PORT_INFORMATION_REQUEST (0x21)  static per-device stub
    PORT_MODE_INFO_REQUEST   (0x22)  static per-mode stub
    PORT_INPUT_FORMAT_SINGLE (0x41)  ack + start value streaming
    PORT_OUTPUT_COMMAND      (0x81)  motor / LED dispatch to hub.port API
    PORT_VALUE_SINGLE        (0x45)  streamed on delta

References:
- LEGO LWP3 spec        https://lego.github.io/lego-ble-wireless-protocol-docs/
- Inventor hub API      https://lego.github.io/MINDSTORMS-Robot-Inventor-hub-API/
- hub.bluetooth.lwp_*   https://lego.github.io/MINDSTORMS-Robot-Inventor-hub-API/mod_bluetooth.html

THIS IS A TEST THAT DOESN'T EVEN WORK!!!
"""

import hub
import sys
import time

# -------------------------------------------------------------------------
# LWP3 constants
# -------------------------------------------------------------------------
MSG_HUB_PROPERTIES              = 0x01
MSG_HUB_ACTIONS                 = 0x02
MSG_HUB_ALERTS                  = 0x03
MSG_HUB_ATTACHED_IO             = 0x04
MSG_GENERIC_ERROR               = 0x05
MSG_PORT_INFORMATION_REQUEST    = 0x21
MSG_PORT_MODE_INFO_REQUEST      = 0x22
MSG_PORT_INPUT_FORMAT_SETUP_SNG = 0x41
MSG_PORT_INFORMATION            = 0x43
MSG_PORT_MODE_INFORMATION       = 0x44
MSG_PORT_VALUE_SINGLE           = 0x45
MSG_PORT_INPUT_FORMAT_SINGLE    = 0x47
MSG_PORT_OUTPUT_COMMAND         = 0x81
MSG_PORT_OUTPUT_FEEDBACK        = 0x82

# Hub property IDs
PROP_ADVERTISING_NAME   = 0x01
PROP_BUTTON             = 0x02
PROP_FW_VERSION         = 0x03
PROP_HW_VERSION         = 0x04
PROP_RSSI               = 0x05
PROP_BATTERY_VOLTAGE    = 0x06
PROP_BATTERY_TYPE       = 0x07
PROP_MANUFACTURER       = 0x08
PROP_RADIO_FW_VERSION   = 0x09
PROP_LWP_VERSION        = 0x0A
PROP_SYSTEM_TYPE_ID     = 0x0B
PROP_HW_NETWORK_ID      = 0x0C
PROP_PRIMARY_MAC        = 0x0D
PROP_SECONDARY_MAC      = 0x0E
PROP_HW_NETWORK_FAMILY  = 0x0F

# Hub property operations
HPO_SET_DOWNSTREAM              = 0x01
HPO_ENABLE_UPDATES_DOWNSTREAM   = 0x02
HPO_DISABLE_UPDATES_DOWNSTREAM  = 0x03
HPO_RESET_DOWNSTREAM            = 0x04
HPO_REQUEST_UPDATE_DOWNSTREAM   = 0x05
HPO_UPDATE_UPSTREAM             = 0x06

# Port output sub-commands (a subset — extend as needed)
SUB_START_POWER              = 0x01
SUB_START_POWER_2            = 0x02
SUB_SET_ACC_TIME             = 0x05
SUB_SET_DEC_TIME             = 0x06
SUB_START_SPEED_SINGLE       = 0x07
SUB_START_SPEED_FOR_TIME_SNG = 0x09
SUB_START_SPEED_FOR_DEG_SNG  = 0x0B
SUB_GOTO_ABS_POS_SINGLE      = 0x0D
SUB_WRITE_DIRECT_MODE        = 0x51

# IOEvent (byte 4 of HUB_ATTACHED_IO)
IO_DETACHED = 0x00
IO_ATTACHED = 0x01

# Hub action
HA_DISCONNECT       = 0x02
HA_SWITCH_OFF_HUB   = 0x01
HA_FAST_POWER_DOWN  = 0x2F

# -------------------------------------------------------------------------
# Transport
# -------------------------------------------------------------------------
_conn_id = None
_hub_prop_enabled = {}         # {prop_id: bool}   updates streaming
_alert_enabled = {}            # {alert_type: bool}
_input_setup_single = {}       # {port_id: {mode: (delta, notify)}}
_last_value_sent_at = {}       # {(port_id, mode): timestamp_ms}

# Cached hub property values. Filled at start() and updated when we learn
# something. Every entry is (op-suffix bytes) — the raw payload placed after
# the property-id + UPDATE_UPSTREAM op.
_hub_prop_value = {}


def _write_frame(payload_after_type: bytes, msg_type: int) -> None:
    """Wrap `payload_after_type` with LWP3 header and push to peer.

    LWP3 frame layout: [length, hub_id=0, msg_type, payload...]
    Length uses the 1-byte form for messages <= 127 bytes (adequate here).
    """
    if _conn_id is None:
        return
    body = bytes(payload_after_type)
    total = 3 + len(body)
    if total <= 127:
        frame = bytes([total, 0x00, msg_type]) + body
    else:
        # Extended-length encoding: bit7 of first byte = 1, second byte = high 7 bits
        total += 1
        frame = bytes([(total & 0x7F) | 0x80, (total >> 7) & 0xFF, 0x00, msg_type]) + body
    try:
        sys.stdout.buffer.write(frame)
    except Exception as e:
        print("send failed:", e)


def _write_generic_error(cmd_type: int, err_code: int) -> None:
    _write_frame(bytes([cmd_type, err_code]), MSG_GENERIC_ERROR)


# -------------------------------------------------------------------------
# HUB_PROPERTIES (0x01)
# -------------------------------------------------------------------------
def _fill_default_hub_props() -> None:
    _hub_prop_value[PROP_ADVERTISING_NAME]  = b"InventorBridge"
    _hub_prop_value[PROP_BUTTON]            = bytes([0])
    _hub_prop_value[PROP_FW_VERSION]        = bytes([0, 0, 0x02, 0x10])  # 1.2.0.0-ish
    _hub_prop_value[PROP_HW_VERSION]        = bytes([0, 0, 0x08, 0x00])
    _hub_prop_value[PROP_RSSI]              = bytes([0xC0])  # -64
    _hub_prop_value[PROP_BATTERY_VOLTAGE]   = bytes([100])
    _hub_prop_value[PROP_BATTERY_TYPE]      = bytes([0])
    _hub_prop_value[PROP_MANUFACTURER]      = b"LEGO System A/S"
    _hub_prop_value[PROP_RADIO_FW_VERSION]  = b"2_02_00_00"
    _hub_prop_value[PROP_LWP_VERSION]       = bytes([0x00, 0x03])
    _hub_prop_value[PROP_SYSTEM_TYPE_ID]    = bytes([0x81])  # Inventor
    _hub_prop_value[PROP_HW_NETWORK_ID]     = bytes([0])
    _hub_prop_value[PROP_PRIMARY_MAC]       = bytes(6)
    _hub_prop_value[PROP_SECONDARY_MAC]     = bytes(6)
    _hub_prop_value[PROP_HW_NETWORK_FAMILY] = bytes([0])


def _refresh_dynamic_hub_props() -> None:
    """Pull live values from hub API where available."""
    try:
        mv = hub.battery.voltage()
        _hub_prop_value[PROP_BATTERY_VOLTAGE] = bytes([max(0, min(100, mv // 100))])
    except Exception:
        pass


def _send_hub_property_update(prop_id: int) -> None:
    if prop_id not in _hub_prop_value:
        _write_generic_error(MSG_HUB_PROPERTIES, 0x05)
        return
    _refresh_dynamic_hub_props()
    payload = bytes([prop_id, HPO_UPDATE_UPSTREAM]) + _hub_prop_value[prop_id]
    _write_frame(payload, MSG_HUB_PROPERTIES)


def _handle_hub_properties(msg: bytes) -> None:
    if len(msg) < 5:
        return
    prop_id = msg[3]
    op = msg[4]
    if op == HPO_REQUEST_UPDATE_DOWNSTREAM:
        _send_hub_property_update(prop_id)
    elif op == HPO_SET_DOWNSTREAM:
        _hub_prop_value[prop_id] = bytes(msg[5:])
    elif op == HPO_ENABLE_UPDATES_DOWNSTREAM:
        _hub_prop_enabled[prop_id] = True
    elif op == HPO_DISABLE_UPDATES_DOWNSTREAM:
        _hub_prop_enabled[prop_id] = False
    elif op == HPO_RESET_DOWNSTREAM:
        _fill_default_hub_props()
    else:
        _write_generic_error(MSG_HUB_PROPERTIES, 0x05)


# -------------------------------------------------------------------------
# HUB_ACTIONS (0x02)
# -------------------------------------------------------------------------
def _handle_hub_actions(msg: bytes) -> None:
    if len(msg) < 4:
        return
    action = msg[3]
    if action in (HA_SWITCH_OFF_HUB, HA_DISCONNECT, HA_FAST_POWER_DOWN):
        try:
            hub.power_off()
        except Exception:
            pass


# -------------------------------------------------------------------------
# HUB_ALERTS (0x03)
# -------------------------------------------------------------------------
def _handle_hub_alerts(msg: bytes) -> None:
    if len(msg) < 5:
        return
    alert_type = msg[3]
    alert_op = msg[4]
    if alert_op == 0x01:
        _alert_enabled[alert_type] = True
    elif alert_op == 0x02:
        _alert_enabled[alert_type] = False
    elif alert_op == 0x03:
        # 0x00 = OK / no alert. Send back status.
        _write_frame(bytes([alert_type, 0x04, 0x00]), MSG_HUB_ALERTS)


# -------------------------------------------------------------------------
# PORT_INFORMATION_REQUEST (0x21) — static stub for now
# -------------------------------------------------------------------------
def _handle_port_info_request(msg: bytes) -> None:
    if len(msg) < 5:
        return
    port_id = msg[3]
    info_type = msg[4]
    if info_type == 0x01:
        # capabilities=input+output, 1 mode, input mask=0x01, output mask=0x01
        _write_frame(bytes([port_id, info_type, 0x03, 0x01, 0x01, 0x00, 0x01, 0x00]),
                     MSG_PORT_INFORMATION)
    elif info_type == 0x02:
        _write_frame(bytes([port_id, info_type, 0x00, 0x00]), MSG_PORT_INFORMATION)


# -------------------------------------------------------------------------
# PORT_MODE_INFORMATION_REQUEST (0x22) — static stub
# -------------------------------------------------------------------------
def _handle_port_mode_info_request(msg: bytes) -> None:
    if len(msg) < 6:
        return
    port_id, mode, info_type = msg[3], msg[4], msg[5]
    payload = bytes([port_id, mode, info_type])
    if info_type == 0x00:      # NAME
        payload += b"MODE0\x00"
    elif info_type == 0x01:    # RAW range
        payload += bytes(8)
    elif info_type == 0x02:    # PCT range
        payload += bytes(8)
    elif info_type == 0x03:    # SI range
        payload += bytes(8)
    elif info_type == 0x04:    # symbol
        payload += b"n/a\x00"
    elif info_type == 0x05:    # mapping
        payload += bytes([0x50, 0x00])
    elif info_type == 0x80:    # VALUE_FORMAT: 1 dataset, 8-bit, 3 figures, 0 decimals
        payload += bytes([1, 0, 3, 0])
    else:
        return
    _write_frame(payload, MSG_PORT_MODE_INFORMATION)


# -------------------------------------------------------------------------
# PORT_INPUT_FORMAT_SETUP_SINGLE (0x41)
# -------------------------------------------------------------------------
def _handle_input_format_setup_single(msg: bytes) -> None:
    if len(msg) < 10:
        return
    port_id = msg[3]
    mode = msg[4]
    delta = msg[5] | (msg[6] << 8) | (msg[7] << 16) | (msg[8] << 24)
    notify = bool(msg[9])
    _input_setup_single.setdefault(port_id, {})[mode] = (delta, notify)
    # Acknowledge with PORT_INPUT_FORMAT_SINGLE (0x47)
    _write_frame(msg[3:10], MSG_PORT_INPUT_FORMAT_SINGLE)


# -------------------------------------------------------------------------
# PORT_OUTPUT_COMMAND (0x81) — dispatch to hub port API
# -------------------------------------------------------------------------
_PORT_LETTERS = "ABCDEF"


def _hub_port(port_id: int):
    if port_id >= len(_PORT_LETTERS):
        return None
    try:
        return getattr(hub.port, _PORT_LETTERS[port_id])
    except Exception:
        return None


def _handle_port_output(msg: bytes) -> None:
    if len(msg) < 6:
        return
    port_id  = msg[3]
    # startup_completion = msg[4]
    sub_cmd  = msg[5]
    payload  = msg[6:]
    p = _hub_port(port_id)

    def _sbyte(b):
        return b - 256 if b > 127 else b

    try:
        if sub_cmd == SUB_START_POWER and p and p.motor:
            p.motor.pwm(_sbyte(payload[0]))
        elif sub_cmd == SUB_START_SPEED_SINGLE and p and p.motor:
            # payload: speed(i8), maxPower(u8), useProfile(u8)
            p.motor.pwm(_sbyte(payload[0]))
        elif sub_cmd == SUB_START_SPEED_FOR_TIME_SNG and p and p.motor:
            duration_ms = payload[0] | (payload[1] << 8)
            speed       = _sbyte(payload[2])
            # max_power  = payload[3]; braking = payload[4]
            p.motor.run_for_time(duration_ms, speed)
        elif sub_cmd == SUB_START_SPEED_FOR_DEG_SNG and p and p.motor:
            degrees = (payload[0] | (payload[1] << 8) |
                       (payload[2] << 16) | (payload[3] << 24))
            speed   = _sbyte(payload[4])
            p.motor.run_for_degrees(degrees, speed)
        elif sub_cmd == SUB_GOTO_ABS_POS_SINGLE and p and p.motor:
            pos = (payload[0] | (payload[1] << 8) |
                   (payload[2] << 16) | (payload[3] << 24))
            if pos & 0x80000000:
                pos -= 0x100000000
            speed = _sbyte(payload[4])
            p.motor.run_to_position(pos, speed)
        elif sub_cmd == SUB_SET_ACC_TIME and p and p.motor:
            ms = payload[0] | (payload[1] << 8)
            p.motor.default(acceleration=ms)
        elif sub_cmd == SUB_SET_DEC_TIME and p and p.motor:
            ms = payload[0] | (payload[1] << 8)
            p.motor.default(deceleration=ms)
        elif sub_cmd == SUB_WRITE_DIRECT_MODE:
            # payload: [mode, data...] — LED colour on hub LED (port 50 typical)
            if p and hasattr(p, "device") and p.device:
                p.device.mode(payload[0], bytes(payload[1:]))
        else:
            pass
    except Exception as e:
        print("output cmd err:", e, sub_cmd)

    # Immediate BUFFER_EMPTY + COMPLETED feedback.
    _write_frame(bytes([port_id, 0x0A]), MSG_PORT_OUTPUT_FEEDBACK)


# -------------------------------------------------------------------------
# Attached-IO enumeration on connect
# -------------------------------------------------------------------------
def _enumerate_attached_io() -> None:
    for i, letter in enumerate(_PORT_LETTERS):
        try:
            p = getattr(hub.port, letter)
            info = p.info()
            dev_type = info.get("type") if isinstance(info, dict) else None
        except Exception:
            dev_type = None
        if dev_type is None:
            continue
        payload = bytes([i, IO_ATTACHED, dev_type & 0xFF, 0x00,
                         0, 0, 0x02, 0x10, 0, 0, 0x08, 0x00])
        _write_frame(payload, MSG_HUB_ATTACHED_IO)


# -------------------------------------------------------------------------
# Value streaming
# -------------------------------------------------------------------------
def _stream_values() -> None:
    now = time.ticks_ms()
    for port_id, modes in _input_setup_single.items():
        p = _hub_port(port_id)
        if not p:
            continue
        try:
            reading = p.device.get() if p.device else None
        except Exception:
            reading = None
        if reading is None:
            continue
        for mode, (delta, notify) in modes.items():
            if not notify:
                continue
            key = (port_id, mode)
            if time.ticks_diff(now, _last_value_sent_at.get(key, 0)) < 100:
                continue
            _last_value_sent_at[key] = now
            data = reading[0] if isinstance(reading, (list, tuple)) and reading else 0
            if not isinstance(data, int):
                try:
                    data = int(data)
                except Exception:
                    data = 0
            _write_frame(bytes([port_id, data & 0xFF]), MSG_PORT_VALUE_SINGLE)


# -------------------------------------------------------------------------
# Dispatcher
# -------------------------------------------------------------------------
def _dispatch(msg: bytes) -> None:
    if len(msg) < 3:
        return
    msg_type = msg[2]
    if msg_type == MSG_HUB_PROPERTIES:
        _handle_hub_properties(msg)
    elif msg_type == MSG_HUB_ACTIONS:
        _handle_hub_actions(msg)
    elif msg_type == MSG_HUB_ALERTS:
        _handle_hub_alerts(msg)
    elif msg_type == MSG_PORT_INFORMATION_REQUEST:
        _handle_port_info_request(msg)
    elif msg_type == MSG_PORT_MODE_INFO_REQUEST:
        _handle_port_mode_info_request(msg)
    elif msg_type == MSG_PORT_INPUT_FORMAT_SETUP_SNG:
        _handle_input_format_setup_single(msg)
    elif msg_type == MSG_PORT_OUTPUT_COMMAND:
        _handle_port_output(msg)
    else:
        _write_generic_error(msg_type, 0x05)


# -------------------------------------------------------------------------
# Byte accumulator — reads from the SPIKE serial channel (sys.stdin.buffer)
# and slices out LWP3 frames using the length byte at offset 0.
# -------------------------------------------------------------------------
def _reader_loop() -> None:
    buf = bytearray()
    while True:
        try:
            chunk = sys.stdin.buffer.read()  # non-blocking on Inventor firmware
        except Exception:
            chunk = None
        if chunk:
            buf.extend(chunk)
            while len(buf) >= 1:
                b0 = buf[0]
                if b0 == 0:
                    buf.pop(0)
                    continue
                if b0 & 0x80:
                    if len(buf) < 2:
                        break
                    total = (b0 & 0x7F) | (buf[1] << 7)
                else:
                    total = b0
                if len(buf) < total:
                    break
                frame = bytes(buf[:total])
                del buf[:total]
                _dispatch(frame)
        _stream_values()
        time.sleep_ms(20)


def _on_connect(conn_id: int) -> None:
    global _conn_id
    _conn_id = conn_id
    print("LWP3 central connected, conn_id =", conn_id)
    _enumerate_attached_io()


def start() -> None:
    _fill_default_hub_props()
    hub.bluetooth.lwp_advertise(0)         # advertise LWP3 UUID indefinitely
    hub.bluetooth.lwp_bypass(True)         # get raw frames instead of built-in handling
    hub.bluetooth.lwp_monitor(_on_connect)
    print("Inventor LWP3 bridge running.")
    _reader_loop()


if __name__ == "__main__":
    start()
