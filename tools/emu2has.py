import re
import sys
import struct

if __name__ == "__main__":
    outFile = open(sys.argv[2], "wb")
    m = {"1": -9, "2": -8, "4": -1}
        
    for l in open(sys.argv[1], "r"):
        #print(l.rstrip())
        p = re.search("Write at 0x([0-9a-f]*) ([0-9]*) 0x([0-9a-f]*)", l.rstrip())       
        #print(l.rstrip())
        if not p: continue
        outFile.write(struct.pack("<lLL", m[p[2]], int(p[1], 16), int(p[3], 16)))