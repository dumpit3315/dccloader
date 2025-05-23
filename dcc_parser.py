import re
import sys

if __name__ == "__main__":
    o = open(sys.argv[2], "wb")
    for l in open(sys.argv[1], encoding="latin-1"):
        m = re.match(r'DCC OCD -> HOST 0x([0-9a-f]*)', l.rstrip())
        if m is None: continue
        print(m)
        s = int(m[1], 16)
        print(s)
        o.write(s.to_bytes(4, "little"))