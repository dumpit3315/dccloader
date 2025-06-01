import sys

if __name__ == "__main__":
    fIn = open(sys.argv[1], "rb+")
    offset = int(sys.argv[2], 16)
    fIn.seek(0x34)
    oof = int.from_bytes(fIn.read(4), "little") + offset

    fIn.seek(0x34)
    fIn.write(oof.to_bytes(4, "little"))
