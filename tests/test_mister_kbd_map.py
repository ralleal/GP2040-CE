#!/usr/bin/env python3
"""Offline binary/firmware contract checks; no MiSTer or network required."""
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class KeyboardMapTests(unittest.TestCase):
    def test_binary_and_firmware_contract(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "config/kbd_cafe_0001.map"
            command = [sys.executable, str(ROOT / "scripts/generate_mister_kbd_map.py"), str(output)]
            subprocess.run(command, check=True)
            data = output.read_bytes()
            subprocess.run(command, check=True)
            self.assertEqual(data, output.read_bytes(), "generation must be reproducible")
        self.assertEqual(len(data), 256)
        # Independent golden bytes: Linux F13..F17 -> P, LShift, LCtrl, LAlt, RAlt.
        self.assertEqual(data, bytes(183) + bytes.fromhex("19 2a 1d 38 64") + bytes(68))

        board = (ROOT / "configs/WaveshareZero/BoardConfig.h").read_text()
        expected = {"SINGLE_KEY_1": "F13", "DOUBLE_KEY_1": "F12",
                    "LONG_KEY_1": "F14", "LONG_KEY_2": "F15",
                    "LONG_KEY_3": "F16", "LONG_KEY_4": "F17"}
        actual = dict(re.findall(r"^#define MISTER_MENU_((?:SINGLE|DOUBLE|LONG)_KEY_\d+)\s+HID_KEY_(\w+)\s*$", board, re.M))
        self.assertEqual(actual, expected)

        # MiSTer applies the sparse map once, after JAMMA; EV_KEY value (up,
        # down, repeat) is unchanged. JAMMA joystick codes >=256 bypass it.
        def translate(code, value):
            return (data[code] or code if code < 256 else code), value

        for value in (0, 1, 2):
            for source, target in ((183, 25), (184, 42), (185, 29), (186, 56), (187, 100)):
                self.assertEqual(translate(source, value), (target, value))
            for code in (88, 2, 3, 6, 7, 0x120, 0x121):  # F12, 1/2/5/6, JAMMA Coin/Start
                self.assertEqual(translate(code, value), (code, value))


if __name__ == "__main__":
    unittest.main()
