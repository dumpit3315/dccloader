import argparse
import os
import pefile
import struct

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("in_file")
    ap.add_argument("out_has")

    args = ap.parse_args()
    pe = pefile.PE(args.in_file)
    
    for sect in pe.sections:
        if sect.Name.startswith(b".data"):
            loader_info = bytearray(pe.get_data(sect.VirtualAddress))
            dcc_signature = loader_info.find(b"&SF1")
            if dcc_signature == -1: dcc_signature = loader_info.find(b"1.00")

            has_init = dcc_signature - 0x15

            unlock_offs, unlock_sz, init_offs, init_sz = struct.unpack("<LLLL", loader_info[has_init:has_init+0x10])
            unlock_offs -= 0x400000 + sect.VirtualAddress
            init_offs -= 0x400000 + sect.VirtualAddress

            base_fn, base_ext = os.path.splitext(args.out_has)

            open(f"{base_fn}_init{base_ext}", "wb").write(loader_info[init_offs:init_offs+init_sz])
            open(f"{base_fn}_unlock{base_ext}", "wb").write(loader_info[unlock_offs:unlock_offs+unlock_sz])

            #open("test.bin", "wb").write(pe.get_data(sect.VirtualAddress))
            #raise Exception()

    # peData = pe.get_data(pe.OPTIONAL_HEADER.Add)

    # open("test.bin", "wb").write(peData)

