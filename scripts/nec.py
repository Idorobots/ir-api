#! /bin/env python3

import sys

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: {} address command".format(sys.argv[0]))
    else:
        address = int(sys.argv[1], 16)
        inv_addr = ~address & 0xFF
        command = int(sys.argv[2], 16)
        inv_cmd = ~command & 0xFF
        print("{:02x}{:02x}{:02x}{:02x}".format(address, inv_addr, command, inv_cmd))
