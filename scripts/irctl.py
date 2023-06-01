#! /bin/env python

import sys
import mitsubishi
import nec
import requests
import time

BASE_URL="192.168.1.222"

def arg_or_default(arg, default):
    if len(sys.argv) > arg:
        return sys.argv[arg]
    else:
        return default

def usage():
    print("Usage: {} device [arguments]".format(sys.argv[0]))
    print("Supported devices: ac, ap, va, moodlight".format(sys.argv[0]))

def run(protocol, code, extra = ""):
    url = "http://{}/ir/send?protocol={}&code={}{}".format(BASE_URL, protocol, code, extra)
    print(url)
    resp = requests.get(url)
    print(resp.content)
    return resp

if __name__ == "__main__":
    if len(sys.argv) < 2:
        usage()
    else:
        device = sys.argv[1]

        # AC
        if device == "ac":
            run("MITSUBISHI_AC",
                mitsubishi.mitsubishi_ac_state(
                    arg_or_default(2, "on"), # power
                    arg_or_default(3, "cool"), # mode
                    arg_or_default(4, "25"), # temp
                    arg_or_default(5, "auto"), # fan
                    arg_or_default(6, "auto"), # vane
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
            for i in range(2, len(sys.argv)):
                run("NEC",
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
            for i in range(2, len(sys.argv)):
                run("RAW_BITS",
                    CMD[sys.argv[i]],
                    "&repeat=5&hmt=8&fmt=0"
                )
                time.sleep(0.2)

        # Moodlight
        elif device == "moodlight":
            ok

        else:
          usage()
