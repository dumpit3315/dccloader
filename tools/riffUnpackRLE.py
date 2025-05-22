import sys

if __name__ == "__main__":
    f = open(sys.argv[1], "rb")
    fo = open(sys.argv[2], "wb")
    
    while True:
        flag = f.read(2)
        if len(flag) == 0: break
        
        flag = int.from_bytes(flag, "little")
        
        count = flag & 0x7fff
        rle = flag >> 15
        
        fo.write((f.read(1) * count) if rle else f.read(count))