import struct
import sys

if __name__ == "__main__":
    f = open(sys.argv[1], "rb")

    while True:
        fp = f.read(4)
        if len(fp) < 4: break

        cmd = int.from_bytes(fp, "little", signed=True)
        if cmd == -1:
            offset, data = struct.unpack("<LL", f.read(8))
            print(f"{cmd} (WRITE): {hex(offset)} {hex(data)}")

        elif cmd == -9:
            offset, data = struct.unpack("<LL", f.read(8))
            print(f"{cmd} (WRITE8): {hex(offset)} {hex(data)}")

        elif cmd == -8:
            offset, data = struct.unpack("<LL", f.read(8))
            print(f"{cmd} (WRITE16): {hex(offset)} {hex(data)}")

        elif cmd == -2:
            offset = int.from_bytes(f.read(4), "little")
            print(f"{cmd} (READ): {hex(offset)}")

        elif cmd in [-3, -21]:
            offset, data = struct.unpack("<LL", f.read(8))
            print(f"{cmd} (AND): v({hex(offset)}) | {hex(data)}")

        elif cmd in [-42, -44]:
            offset, data = struct.unpack("<LL", f.read(8))
            print(f"{cmd} (OR): v({hex(offset)}) & ~{hex(data)}")

        elif cmd == -43:
            offset, mask, data = struct.unpack("<LLL", f.read(12))
            print(f"{cmd} (READ OR): (v({hex(offset)}) & ~{hex(mask)}) | {hex(data)}")

        elif cmd == -250: # Unknown 2
            print(f"{cmd}: {f.read(4)}")

        elif cmd == -19:
            code = int.from_bytes(f.read(4), "little")
            print(f"{cmd} (SKIP), {code}")

        elif cmd == -40:
            code = int.from_bytes(f.read(4), "little")
            print(f"{cmd} (READ_AND_PRINT), {hex(code)}")

        elif cmd == -12: # COPROC
            cr_m, cr_n, cp_no, op, data = struct.unpack("<BBBBL", f.read(8))            
            print(F"{cmd} (COPROC) {cp_no}, {cr_n}, {cr_m}, {op}, {hex(data)}")

        elif cmd == -7: # Write again
            offset, value, mask = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (WRITE AND): {hex(offset)}, ({hex(mask)} | {hex(value)}) = {hex(value | mask)}")
            #print(f"{cmd} (WRITE OR): (v({hex(offset)}) & ~{hex(mask)}) | {hex(value)}")

        elif cmd in [-17, -25]:
            offset, mask, expected, delay, branch = struct.unpack("<LLLLL", f.read(0x14))
            print(f"{cmd} (POLL_TIMEOUT): ({hex(offset)} & {hex(mask)}) == {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TIMEOUT")

        elif cmd in [-18, -26]:
            offset, mask, expected, delay, branch = struct.unpack("<LLLLL", f.read(0x14))
            print(f"{cmd} (POLL): ({hex(offset)} & {hex(mask)}) == {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TRUE")

        elif cmd == -54:
            offset, bits = struct.unpack("<LL", f.read(0x8))
            print(f"{cmd} (READ BYTES and PRINT) {hex(offset)} {bits}")

        elif cmd == -59:
            offset, count, value = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (WRITE8) {hex(offset)} {hex(value)} {count}")

        elif cmd == -58:
            offset, count, value = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (WRITE16) {hex(offset)} {hex(value)} {count}")

        elif cmd == -41:
            reg = int.from_bytes(f.read(4), "little")
            print(f"{cmd} (PRINT): {hex(reg)}")

        elif cmd == -38:
            mask, cond, branch = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if TRUE")

        elif cmd == -39:
            mask, cond, branch = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if FALSE")

        elif cmd == -255:
            print(f"{cmd} (RETURN)")

        else:
            raise Exception(f"command {cmd} {hex(f.tell() - 4)}")