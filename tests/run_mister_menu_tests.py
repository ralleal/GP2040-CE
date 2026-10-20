#!/usr/bin/env python3
"""Compile the production A1 block on the host, with simulated time and input.

No Pico SDK is required. State declarations, timing/key macros, the process block
and shortcut methods are read verbatim from firmware sources, never reimplemented.
This intentionally tests the A1 state machine, not USB delivery or physical GPIO.
"""
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / 'src/drivers/keyboard/KeyboardDriver.cpp').read_text()
header = (ROOT / 'headers/drivers/keyboard/KeyboardDriver.h').read_text()
board = (ROOT / 'configs/WaveshareZero/BoardConfig.h').read_text()


def between(text, start, end):
    assert text.count(start) == 1, f'Expected one source marker: {start}'
    return text.split(start, 1)[1].split(end, 1)[0]


state = between(header, '// State for the custom MiSTer', '\n};')
state = state[state.index('    bool misterMenuRawPressed'):]
block = between(source, '#ifdef MISTER_MENU_MULTI_ACTION_ENABLED\n', '\n#endif')
actions = between(source, 'void KeyboardDriver::pressMisterMenuSingleAction()', '\nvoid KeyboardDriver::pressKey(')
macros = '\n'.join(re.findall(r'^#define MISTER_MENU_.*$', board, re.MULTILINE))
# Keep all production action bodies, changing only the surrounding class name.
actions = 'void KeyboardDriver::pressMisterMenuSingleAction()' + actions
harness = '''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <set>
constexpr uint8_t HID_KEY_F12 = 0x45, HID_KEY_F13 = 0x68,
    HID_KEY_F14 = 0x69, HID_KEY_F15 = 0x6a, HID_KEY_F16 = 0x6b, HID_KEY_F17 = 0x6c;
''' + macros + '''
static uint32_t clockMs;
uint32_t get_absolute_time() { return clockMs; }
uint32_t to_ms_since_boot(uint32_t time) { return time; }
struct Gamepad {
    bool down;
    bool pressedA1() const { return down; }
};
class KeyboardDriver {
public:
''' + state + '''
    std::set<uint8_t> keys;
    void pressKey(uint8_t key) { keys.insert(key); }
    void pressMisterMenuSingleAction();
    void pressMisterMenuDoubleAction();
    void pressMisterMenuLongAction();
    void process(Gamepad* gamepad) {
        keys.clear();
''' + block + '''
    }
};
''' + actions
with tempfile.TemporaryDirectory(prefix='mister-tests-') as tmp:
    tmp = Path(tmp)
    (tmp / 'mister_menu_production.h').write_text(harness)
    binary = tmp / 'mister_menu_tests'
    subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++17', '-Wall',
                    '-Wextra', '-Werror', '-pedantic', '-I', str(tmp),
                    str(ROOT / 'tests/mister_menu_test.cpp'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
