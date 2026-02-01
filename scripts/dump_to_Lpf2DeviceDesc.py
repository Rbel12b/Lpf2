import re
import sys
from pathlib import Path

FORMAT_MAP = {
    "0x00": "DATA8",
    "0x01": "DATA16",
    "0x02": "DATA32",
    "0x03": "DATAF",
}

def parse_flags(line):
    # line: "Flags: 0x040500000000 (....)"
    hexval = line.split()[1]  # 0x040500000000
    value = int(hexval, 16)

    # Convert to 6 bytes, big-endian
    bytes_be = value.to_bytes(6, byteorder="big")

    return ", ".join(f"0x{b:02X}" for b in bytes_be)

def parse_float(line):
    return float(line.split(":")[1].strip())

def parse_int(line):
    return int(line.split(":")[1].strip(), 0)

def parse_str(line):
    return line.split(":", 1)[1].strip()

def emit_mode(m):
    fmt = FORMAT_MAP.get(m["format"], "DATA8")

    return f"""        {{
            "{m['name']}",
            {m['min']}f, {m['max']}f,
            {m['pct_min']}f, {m['pct_max']}f,
            {m['si_min']}f, {m['si_max']}f,
            "{m['unit']}",
            {m['in']}, {m['out']},
            {m['datasets']}, {fmt}, {m['figures']}, {m['decimals']},
            {{}},
            0x00,
            Lpf2Mode::Flags{{{{ {m['flags']} }}}}
        }}"""

def main(path):
    text = Path(path).read_text()
    lines = iter(text.splitlines())

    devices = []
    current_device = None
    current_mode = None

    for line in lines:
        line = line.strip()
        if line.startswith("Device:"):
            if current_device:
                devices.append(current_device)
            current_device = {
                "id": int(line.split()[1], 0),
                "inModes": 0,
                "outModes": 0,
                "caps": 0,
                "modes": []
            }
        elif line.startswith("InModes:"):
            current_device["inModes"] = parse_int(line)
        elif line.startswith("OutModes:"):
            current_device["outModes"] = parse_int(line)
        elif line.startswith("Caps:"):
            current_device["caps"] = parse_int(line)
        elif line.startswith("Mode"):
            current_mode = {}
            current_device["modes"].append(current_mode)
        elif current_mode is not None:
            if line.startswith("name:"):
                current_mode["name"] = parse_str(line)
            elif line.startswith("unit:"):
                current_mode["unit"] = parse_str(line)
            elif line.startswith("min:"):
                current_mode["min"] = parse_float(line)
            elif line.startswith("max:"):
                current_mode["max"] = parse_float(line)
            elif line.startswith("PCT min:"):
                current_mode["pct_min"] = parse_float(line)
            elif line.startswith("PCT max:"):
                current_mode["pct_max"] = parse_float(line)
            elif line.startswith("SI min:"):
                current_mode["si_min"] = parse_float(line)
            elif line.startswith("SI max:"):
                current_mode["si_max"] = parse_float(line)
            elif line.startswith("Data sets:"):
                current_mode["datasets"] = int(parse_str(line))
            elif line.startswith("format:"):
                current_mode["format"] = parse_str(line)
            elif line.startswith("Figures:"):
                current_mode["figures"] = int(parse_str(line))
            elif line.startswith("Decimals:"):
                current_mode["decimals"] = int(parse_str(line))
            elif line.startswith("in:"):
                current_mode["in"] = line.split()[1]
            elif line.startswith("out:"):
                current_mode["out"] = line.split()[1]
            elif line.startswith("Flags:"):
                current_mode["flags"] = parse_flags(line)

    if current_device:
        devices.append(current_device)

    # ---- EMIT C++ ----
    for dev in devices:
        print(f"// Device 0x{dev['id']:02X}")
        print(f"const Lpf2DeviceDescriptor LPF2_DEVICE_0x{dev['id']:02X} =")
        print("{")
        print(f"    .type = static_cast<Lpf2DeviceType>(0x{dev['id']:02X}),")
        print(f"    .inModes = 0x{dev['inModes']:04X},")
        print(f"    .outModes = 0x{dev['outModes']:04X},")
        print(f"    .caps = 0x{dev['caps']:02X},")
        print(f"    .combos = {{ 0x0000 }},")
        print("    .modes =")
        print("    {")
        for m in dev["modes"]:
            print(emit_mode(m) + ",")
        print("    }")
        print("};\n")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python dump_to_Lpf2DeviceDesc.py dump.txt")
        sys.exit(1)
    main(sys.argv[1])
