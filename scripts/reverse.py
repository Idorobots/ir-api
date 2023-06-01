#! /bin/env python3

import sys
import json
import textwrap

def close_enough(x, y):
    return abs(x - y) < 0.1

def analyze(data, timings):
    print("Timings: ")
    print(timings)
    print(data)

    if len(timings) < 6:
        raise "Too little data, need at least 6 timings!"

    hmt = timings[0]
    hst = timings[1]

    bit_times = timings[2:]

    marks = bit_times[0::2]
    omt = zmt = min(marks)

    if close_enough(hmt, omt) or close_enough(hst, omt):
        # Looks like there's no header.
        hmt = hst = 0
        bit_times = timings
        marks = bit_times[0::2]
        omt = zmt = min(marks)

    spaces = bit_times[1::2]
    ost = max(spaces)
    zst = min(spaces)

    tick = min([omt, zmt, ost, zst])
    ticks = [round(x / tick) for x in timings]
    ticks_str = ''.join("{:x}".format(x) for x in ticks)

    hmt = round(hmt / tick)
    hst = round(hst / tick)
    omt = round(omt / tick)
    ost = round(ost / tick)
    zmt = round(zmt / tick)
    zst = round(zst / tick)

    print("Ticks:")
    print("tick={}".format(tick))
    print("hmt={}".format(hmt))
    print("hst={}".format(hst))
    print("omt={}".format(omt))
    print("ost={}".format(ost))
    print("zmt={}".format(zmt))
    print("zst={}".format(zst))
    print(ticks)
    print(ticks_str)

    one = "{:x}{:x}".format(omt, ost)
    zero = "{:x}{:x}".format(zmt, zst)

    bits = [ b == one and 1 or 0 for b in textwrap.wrap(ticks_str[2:], 2)]
    bits_str = ''.join(str(x) for x in bits)

    print("Bits:")
    print(bits)
    print(bits_str)
    print(hex(int(bits_str, 2)))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <data-to-reverse-engineer> [repeats]".format(sys.argv[0]))
    else:
        try:
            obj = json.loads(sys.argv[1])
            data = obj["code"]
        except:
            data = sys.argv[1]

        print("Data:")
        print(data)

        repeats = len(sys.argv) > 2 and int(sys.argv[2]) or 1
        print("Expecting {} chunks".format(repeats))

        chunk_len = round(len(data) / repeats + 0.5)
        chunks = textwrap.wrap(data, chunk_len)

        print("Chunks: ")
        print(chunks)

        for chunk in chunks:
            print()
            print("Chunk {}".format(chunk))

            timings = [int(x, 16) for x in textwrap.wrap(chunk, 4)]
            analyze(chunk, timings)
