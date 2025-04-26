import re
import sys
import struct

if __name__ == "__main__":
    outFile = open(sys.argv[2], "wb")
            
    for l in open(sys.argv[1], "r"):
        #print(l.rstrip())
        p = re.search(r"([0-9-]*) \(([\S]*)\): ([\S]*) ([\S]*)", l.rstrip())       
        #print(l.rstrip())
        if not p: continue
        outFile.write(struct.pack("<lLL", int(p[1]), int(p[3], 16), int(p[4], 16)))