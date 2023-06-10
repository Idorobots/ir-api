#! /bin/env python

import sys
import mitsubishi
import nec
import requests
import time

def arg_or_default(arg, default):
    if len(sys.argv) > arg:
        return sys.argv[arg]
    else:
        return default

def usage():
    print("Usage: {} host device [arguments]".format(sys.argv[0]))
    print("Supported devices: ac, ap, va, moodlight".format(sys.argv[0]))

def run(host, protocol, code, extra = ""):
    url = "http://{}/ir/send?protocol={}&code={}{}".format(host, protocol, code, extra)
    print(url)
    resp = requests.get(url)
    print(resp.content)
    return resp

if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()
    else:
        host = sys.argv[1]
        device = sys.argv[2]

        # AC
        if device == "ac":
            run(host,
                "MITSUBISHI_AC",
                mitsubishi.mitsubishi_ac_state(
                    arg_or_default(3, "on"), # power
                    arg_or_default(4, "cool"), # mode
                    arg_or_default(5, "25"), # temp
                    arg_or_default(6, "auto"), # fan
                    arg_or_default(7, "auto"), # vane
                ),
                "&repeat=2"
            )

        # BlueAir purifier
        elif device == "ap":
            CMD = {
                "on": "00",
                "off": "00",
                "speed": "20",
            }
            for i in range(3, len(sys.argv)):
                run(host,
                    "NEC",
                    nec.nec_state("00", CMD[sys.argv[i]]),
                    "&repeat=4"
                )
                time.sleep(0.2)

        # Robot vacuum
        elif device == "vac":
            CMD = {
                "start": "54",
                "pause": "54",
                "home": "12",
                "zigzag": "aa",
                "spot": "be",
                "corners": "dc",
                "left": "76",
                "forward": "36",
                "right": "8c"
            }
            for i in range(3, len(sys.argv)):
                run(host,
                    "RAW_BITS",
                    CMD[sys.argv[i]],
                    "&repeat=5&hmt=8&fmt=0"
                )
                time.sleep(0.2)

        # Moodlight
        elif device == "moodlight":
            CMD = {
                "on": "e0",
                "off": "60",
                "+": "a0",
                "-": "20",
                "fade": "d8",
                "strobe": "e8",
                "flash": "f0",
                "smooth": "c8",
                "r": "90",
                "g": "10",
                "b": "50",
                "w": "d0",
                "r1": "b0",
                "r2": "a8",
                "r3": "98",
                "r4": "88",
                "g1": "30",
                "g2": "28",
                "g3": "18",
                "g4": "08",
                "b1": "70",
                "b2": "68",
                "b3": "58",
                "b4": "48"
            }
            for i in range(3, len(sys.argv)):
                run(host,
                    "NEC",
                    nec.nec_state("00", CMD[sys.argv[i]]),
                    "&repeat=4"
                )
                time.sleep(0.2)

        else:
          usage()
