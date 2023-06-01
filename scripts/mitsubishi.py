#! /bin/env python

import sys
import textwrap

MODE = {
    "auto": "100",
    "cool": "011",
    "dry": "010",
    "heat": "001",
    "fan": "111"
}

MODE_RAW = {
    "auto": "00110000",
    "cool": "00110110",
    "dry": "00110010",
    "heat": "00110000",
    "fan": "00110111"
}

FAN_MODE = {
    "auto": "000",
    "1": "001",
    "2": "010",
    "3": "011",
    "4": "100",
    "silent": "110",
}

VANE_MODE = {
    "auto": "000",
    "highest": "001",
    "high": "010",
    "middle": "011",
    "low": "100",
    "lowest": "101",
    "swing": "111"
}

def mitsubishi_ac_state(p, m, t, f, v):
    power = p == "on" and "00100000" or "00000000"

    # TODO econocool
    mode = MODE[m]
    mode_raw = MODE_RAW[m]

    clamped_temp = max(min(float(t), 31.0), 16.0)
    temp = "{:04b}".format(int(clamped_temp - 16.0) & 0xff)

    fan = FAN_MODE[f]
    fan_auto = f == "auto" and "1" or "0"

    vane = VANE_MODE[v]

    state_bits = power + ("00" + mode + "000") + ("0000" + temp) + mode_raw + (fan_auto + "1" + vane + fan) + ("0" * 8) * 7
    state_bits_bytes = textwrap.wrap(state_bits, 8)

    header = [0x23, 0xcb, 0x26, 0x01, 0x00]
    state_bytes = header + [int(x, 2) for x in state_bits_bytes]
    state_bytes_str = ["{:02x}".format(x) for x in state_bytes]

    checksum = sum(state_bytes) & 0xff
    return "".join(state_bytes_str) + "{:02x}".format(checksum)

if __name__ == "__main__":
    if len(sys.argv) < 6:
        print("Usage: {} power mode temp fan vane".format(sys.argv[0]))
    else:
        if sys.argv[2] not in MODE:
            print("Mode needs to be one of: auto, cool, dry, heat, fan")

        if sys.argv[4] not in FAN_MODE:
            print("Fan needs to be one of: auto, 1, 2, 3, 4, silent")

        if sys.argv[5] not in VANE_MODE:
            print("Vane needs to be one of: auto, highest, high, middle, low, lowest, swing")

        state = mitsubishi_ac_state(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
        print(state)
