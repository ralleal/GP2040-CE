#!/usr/bin/env python3
"""Generate MiSTer's sparse 256-byte Linux EV_KEY translation table.

See README_MiSTerCabinet-Custom.md for the pinned upstream format references.
No HID usages, header, integer packing or host byte order are involved.
"""
import argparse
from pathlib import Path

# Linux input-event-codes.h: source EV_KEY -> destination EV_KEY.
REMAP = {183: 25, 184: 42, 185: 29, 186: 56, 187: 100}


def generate_map():
    data = bytearray(256)  # Zero means pass through, not KEY_RESERVED.
    for source, target in REMAP.items():
        data[source] = target
    return bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="output kbd_cafe_0001.map path")
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(generate_map())


if __name__ == "__main__":
    main()
