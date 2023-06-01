#! /bin/env python3

import sys

def nec_state(a, c):
    address = int(a, 16)
    inv_addr = ~address & 0xFF
    command = int(c, 16)
    inv_cmd = ~command & 0xFF
    return "{:02x}{:02x}{:02x}{:02x}".format(address, inv_addr, command, inv_cmd)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: {} address command".format(sys.argv[0]))
    else:
        state = nec_state(sys.argv[1], sys.argv[2])
        print(state)
