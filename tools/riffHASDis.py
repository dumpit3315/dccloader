import struct
import sys

if __name__ == "__main__":
    f = open(sys.argv[1], "rb")

    while True:
        fp = f.read(4)
        if len(fp) < 4: break

        cmd = int.from_bytes(fp, "little")
        cmd ^= 0xffffffff

        
        match cmd:
            case 0x00: # 0xff
                offset, value = struct.unpack("<LL", f.read(8))
                print(f"{cmd} (WRITE): {hex(offset)} = {hex(value)}")

            case 0x01: # 0xfe
                offset = int.from_bytes(f.read(4), "little")
                print(f"{cmd} (READ): {hex(offset)}")

            case 0x02: # 0xfd
                offset, value = struct.unpack("<LL", f.read(8))
                print(f"{cmd} (WRITE AND): {hex(offset)} |= {hex(value)}")

            case 0x03: # 0xfc
                offset, mask, value = struct.unpack("<LLL", f.read(12))
                print(f"{cmd} (WRITE AND OR): {hex(offset)} |= ({hex(value)} & {hex(mask)})")

            case 0x05: # 0xfa
                unknown = int.from_bytes(f.read(4), "little")
                print(f"{cmd} (UNKNOWN): {hex(unknown)}")

            case 0x06: # 0xf9
                offset, mask, value = struct.unpack("<LLL", f.read(0xc))
                print(f"{cmd} (WRITE OR AND): {hex(offset)} = (v({hex(offset)}) & {hex(mask)}) | {hex(value)}")

            case 0x07: # 0xf8
                offset, value = struct.unpack("<LL", f.read(8))
                print(f"{cmd} (WRITE16): {hex(offset)} = {hex(value)}")

            case 0x08: # 0xf7
                offset, value = struct.unpack("<LL", f.read(8))
                print(f"{cmd} (WRITE8): {hex(offset)} = {hex(value)}")

            case 0x09: # 0xf6
                offset = int.from_bytes(f.read(4), "little")
                print(f"{cmd} (READ16): {hex(offset)}")

            case 0x0a: # 0xf5
                offset = int.from_bytes(f.read(4), "little")
                print(f"{cmd} (READ8): {hex(offset)}")

            case 0x0b: # 0xf4
                cr_m, cr_n, cp_no, op, value = struct.unpack("<BBBBL", f.read(8))            
                print(F"{cmd} (COPROC) {cp_no}, {cr_n}, {cr_m}, {op}, {hex(value)}")

            case 0x0c: # 0xf3
                cr_m, cr_n, cp_no, op, value = struct.unpack("<BBBBL", f.read(8))            
                print(F"{cmd} (COPROC OR) {cp_no}, {cr_n}, {cr_m}, {op}, &= {hex(value)}")

            case 0x0d: # 0xf2
                cr_m, cr_n, cp_no, op, value = struct.unpack("<BBBBL", f.read(8))            
                print(F"{cmd} (COPROC AND) {cp_no}, {cr_n}, {cr_m}, {op}, |= {hex(value)}")

            case 0x0f: # 0xf0
                val = int.from_bytes(f.read(4), "little")
                print(f"{cmd} (SLEEP): {val}")

            case 0x10: # 0xef
                offset, mask, expected, delay, branch = struct.unpack("<LLLLL", f.read(0x14))
                print(f"{cmd} (POLL_TIMEOUT): (v({hex(offset)}) & {hex(mask)}) == {hex(expected)}, max: {delay}ms, SKIP {branch} INSTRUCTION if TIMEOUT")

            case 0x11: # 0xee
                offset, mask, expected, delay, branch = struct.unpack("<LLLLL", f.read(0x14))
                print(f"{cmd} (POLL): (v({hex(offset)}) & {hex(mask)}) == {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TRUE")

            case 0x12: # 0xed
                code = int.from_bytes(f.read(4), "little")
                print(f"{cmd} (SKIP) {code} instructions")

            case 0x18: # 0xe7
                offset, mask = struct.unpack("<LL", f.read(0x8))
                print(f"{cmd} (UNKNOWN) {hex(offset)} {hex(mask)}")

            case 0x25: # 0xda
                mask, cond, branch = struct.unpack("<LLL", f.read(0xc))
                print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if TRUE")

            case 0x26: # 0xd9
                mask, cond, branch = struct.unpack("<LLL", f.read(0xc))
                print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if FALSE")

            case 0x27: # 0xd7
                offset = int.from_bytes(f.read(4), "little")
                print(f"{cmd}: (READ AND PRINT) {hex(offset)}")

            case 0x28: # 0xd7
                val = int.from_bytes(f.read(4), "little")
                print(f"{cmd}: (PRINT) {hex(val)}")

            case 0x29: # 0xd6
                offset, value = struct.unpack("<LL", f.read(8))
                print(f"{cmd} (WRITE OR): {hex(offset)} &= {hex(value)}")

            case 0x2a: # 0xd5
                offset, mask, value = struct.unpack("<LLL", f.read(0xc))
                print(f"{cmd} (UNKNOWN OR AND): {hex(offset)}) ; ({hex(value)} & {hex(mask)})")

            case 0xf9: # 0x06
                val = int.from_bytes(f.read(4), "little")
                print(f"{cmd}: (SET) {val}")

            case 0xfe: # 0x01
                print(f"{cmd} (RETURN)")

            case _:
                raise Exception(f"command {cmd} {hex(f.tell() - 4)}")

        # Bitwise operations
        # elif cmd in [-3, -21]:
        #     offset, value = struct.unpack("<LL", f.read(8))
        #     print(f"{cmd} (WRITE AND): {hex(offset)} |= {hex(value)}")

        # elif cmd in [-42, -44]:
        #     offset, value = struct.unpack("<LL", f.read(8))
        #     print(f"{cmd} (WRITE OR): {hex(offset)} &= ~{hex(value)}")

        # elif cmd == -43:
        #     offset, mask, value = struct.unpack("<LLL", f.read(12))
        #     print(f"{cmd} (READ OR AND): (v({hex(offset)}) & ~{hex(mask)}) | {hex(value)}")

        # elif cmd == -250:
        #     arg = int.from_bytes(f.read(4), "little")
        #     print(f"{cmd}: (SET ARGS) {arg}")

        # elif cmd == -19:
        #     code = int.from_bytes(f.read(4), "little")
        #     print(f"{cmd} (SKIP), {code}")

        # elif cmd == -40:
        #     code = int.from_bytes(f.read(4), "little")
        #     print(f"{cmd} (READ_AND_PRINT), {hex(code)}")

        # elif cmd == -12: # COPROC
        #     cr_m, cr_n, cp_no, op, value = struct.unpack("<BBBBL", f.read(8))            
        #     print(F"{cmd} (COPROC) {cp_no}, {cr_n}, {cr_m}, {op}, {hex(value)}")

        # elif cmd == -12: # COPROC
        #     cr_m, cr_n, cp_no, op, value = struct.unpack("<BBBBL", f.read(8))            
        #     print(F"{cmd} (COPROC OR) {cp_no}, {cr_n}, {cr_m}, {op}, {hex(value)}")

        # elif cmd == -7: # Write again
        #     offset, mask, value = struct.unpack("<LLL", f.read(0xc))
        #     print(f"{cmd} (WRITE OR AND): {hex(offset)} = (v({hex(offset)}) & {hex(mask)}) | {hex(value)}")

        # elif cmd in [-17, -25]:
        #     offset, mask, expected, delay, branch = struct.unpack("<LLLLL", f.read(0x14))
        #     print(f"{cmd} (POLL_TIMEOUT): ({hex(offset)} & {hex(mask)}) == {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TIMEOUT")

        # elif cmd in [-18, -26]:
        #     offset, mask, expected, delay, branch = struct.unpack("<LLLLL", f.read(0x14))
        #     print(f"{cmd} (POLL): ({hex(offset)} & {hex(mask)}) == {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TRUE")

        # elif cmd == -54:
        #     offset, bits = struct.unpack("<LL", f.read(0x8))
        #     print(f"{cmd} (READ BYTES and PRINT) {hex(offset)} {bits}")

        # elif cmd == -59:
        #     offset, count, value = struct.unpack("<LLL", f.read(0xc))
        #     print(f"{cmd} (WRITE8) {hex(offset)} {hex(value)} {count}")

        # elif cmd == -58:
        #     offset, count, value = struct.unpack("<LLL", f.read(0xc))
        #     print(f"{cmd} (WRITE16) {hex(offset)} {hex(value)} {count}")

        # elif cmd == -41:
        #     reg = int.from_bytes(f.read(4), "little")
        #     print(f"{cmd} (PRINT): {hex(reg)}")

        # elif cmd == -38:
        #     mask, cond, branch = struct.unpack("<LLL", f.read(0xc))
        #     print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if TRUE")

        # elif cmd == -39:
        #     mask, cond, branch = struct.unpack("<LLL", f.read(0xc))
        #     print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if FALSE")

        # elif cmd == -255:
        #     print(f"{cmd} (RETURN)")

        # elif cmd == -16:
        #     print(f"{cmd}: {f.read(4)}")
            
        # elif cmd == -65536:
        #     print(f"{cmd}: {f.read(8)}")

        # elif cmd == -6:
        #     value, mask, offset, a1, a2, a3, a4 = struct.unpack("<LLLLLLL", f.read(0x1c))
        #     print(f"{cmd} (UNK): v:{hex(value)}, m:{hex(mask)}, o:{hex(offset)}, a1:{hex(a1)}, a2:{hex(a2)}, a3:{hex(a3)}, a4:{hex(a4)}")

        # elif cmd == -10:
        #     offset = struct.unpack("<L", f.read(0x4))[0]
        #     print(f"{cmd} (UNK) {hex(offset)}")

        # else:
        #     raise Exception(f"command {cmd} {hex(f.tell() - 4)}")