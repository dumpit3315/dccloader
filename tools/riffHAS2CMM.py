import struct
import sys

if __name__ == "__main__":
    f = open(sys.argv[1], "rb")

    ip = 0
    labels = {}
    instr = {}

    while True:
        instr[ip] = []
        
        fp = f.read(4)
        if len(fp) < 4: break

        cmd = int.from_bytes(fp, "little", signed=True)
        if cmd == -1:
            offset, data = struct.unpack("<LL", f.read(8))
            instr[ip].append(f"data.set 0x{offset:08x} %long 0x{data:08x}")
            #print(f"{cmd} (WRITE): {hex(offset)} {hex(data)}")

        elif cmd == -9:
            offset, data = struct.unpack("<LL", f.read(8))
            instr[ip].append(f"data.set 0x{offset:08x} %byte 0x{data:02x}")
            #print(f"{cmd} (WRITE8): {hex(offset)} {hex(data)}")

        elif cmd == -8:
            offset, data = struct.unpack("<LL", f.read(8))
            instr[ip].append(f"data.set 0x{offset:08x} %word 0x{data:04x}")
            #print(f"{cmd} (WRITE16): {hex(offset)} {hex(data)}")

        elif cmd == -2:
            offset = int.from_bytes(f.read(4), "little")
            instr[ip].append(f"&var_a=data.long(0x{offset:08x})")
            #print(f"{cmd} (READ): {hex(offset)}")

        elif cmd in [-3, -21]:
            offset, data = struct.unpack("<LL", f.read(8))
            if cmd == -3:
                instr[ip].append(f"&and_a=data.long(0x{offset:08x})")
                instr[ip].append(f"&and_b=&and_a | (0x{data:08x})")
                instr[ip].append(f"data.set 0x{offset:08x} %long &and_b")
                
            elif cmd == -21:
                instr[ip].append(f"&and_a=data.word(0x{offset:08x})")
                instr[ip].append(f"&and_b=&and_a | (0x{data:04x})")
                instr[ip].append(f"data.set 0x{offset:08x} %word &and_b")
                
            #print(f"{cmd} (AND): v({hex(offset)}) | {hex(data)}")

        elif cmd in [-42, -44]:
            offset, data = struct.unpack("<LL", f.read(8))
            if cmd == -42:
                instr[ip].append(f"&or_a=data.long(0x{offset:08x})")
                instr[ip].append(f"&or_b=&or_a & (~0x{data:08x})")
                instr[ip].append(f"data.set 0x{offset:08x} %long &or_b")
                
            elif cmd == -44:
                instr[ip].append(f"&or_a=data.word(0x{offset:08x})")
                instr[ip].append(f"&or_b=&or_a & (~0x{data:04x})")
                instr[ip].append(f"data.set 0x{offset:08x} %word &or_b")
            #print(f"{cmd} (OR): v({hex(offset)}) & ~{hex(data)}")

        elif cmd == -43:
            offset, mask, data = struct.unpack("<LLL", f.read(12))
            instr[ip].append(f"&or_a=data.long(0x{offset:08x})")
            instr[ip].append(f"&or_b=&or_a & (~0x{mask:08x})")
            
            instr[ip].append(f"&and=&or_b | (0x{data:08x})")
            instr[ip].append(f"data.set 0x{offset:08x} %long &and")
            #print(f"{cmd} (READ OR): (v({hex(offset)}) & ~{hex(mask)}) | {hex(data)}")

        elif cmd == -250: # Unknown 2
            arg = int.from_bytes(f.read(4), "little")
            instr[ip].append(f"&param=0x{arg:08x}")
            #instr[ip].append(f"// {cmd}: {f.read(4)}")

        elif cmd == -19:
            pos = int.from_bytes(f.read(4), "little", signed=True) + 1
            instr[ip].append(f"goto has_{ip+pos}")
            labels[ip+pos] = f"has_{ip+pos}"
            #print(f"{cmd} (SKIP), {pos}")

        elif cmd == -40:
            code = int.from_bytes(f.read(4), "little")
            instr[ip].append(f"&prn_a=data.long(0x{offset:08x})")
            instr[ip].append(f"print &prn_a")
            #print(f"{cmd} (READ_AND_PRINT), {hex(code)}")

        elif cmd == -12: # COPROC
            cr_m, cr_n, cp_no, op, data = struct.unpack("<BBBBL", f.read(8))            
            instr[ip].append(f"data.set c{cp_no}:0x{cr_n:04x} %long 0x{data:08x}")
            #print(F"{cmd} (COPROC) {cp_no}, {cr_n}, {cr_m}, {op}, {hex(data)}")
            

        elif cmd == -7: # Write again
            offset, value, mask = struct.unpack("<LLL", f.read(0xc))
            instr[ip].append(f"&and_a=(0x{mask:08x}) | (0x{value:08x})")
            instr[ip].append(f"data.set 0x{offset:08x} %long &and_a")
            
            #print(f"{cmd} (WRITE AND): {hex(offset)}, ({hex(mask)} | {hex(value)}) = {hex(value | mask)}")
            #print(f"{cmd} (WRITE OR): (v({hex(offset)}) & ~{hex(mask)}) | {hex(value)}")

        elif cmd in [-17, -25]:
            offset, mask, expected, delay, pos = struct.unpack("<LLLLl", f.read(0x14))
            pos += 1
            
            instr[ip].append(f"&assert_i = 0")
            instr[ip].append(f"while &assert_i < {delay + 1}")
            instr[ip].append("{")
            if cmd == -17:
                instr[ip].append(f"    &assert_c = data.long(0x{offset:08x})")
                instr[ip].append(f"    if (&assert_c & 0x{mask:08x}) == 0x{expected:08x}")
                instr[ip].append(f"        goto has_{ip+pos}_true")
                instr[ip].append(f"    &assert_i = &assert_i + 1")
                labels[ip+pos] = f"has_{ip+pos}"
                
            elif cmd == -25:
                instr[ip].append(f"    &assert_c = data.word(0x{offset:08x})")
                instr[ip].append(f"    if (&assert_c & 0x{mask:04x}) == 0x{expected:04x}")    
                instr[ip].append(f"        goto has_{ip+pos}_true")
                instr[ip].append(f"    &assert_i = &assert_i + 1")
                labels[ip+pos] = f"has_{ip+pos}"
            
            instr[ip].append("}")
            instr[ip].append(f"goto has_{ip+pos}")
            instr[ip].append(f"has_{ip+pos}_true:")
            #print(f"{cmd} (READ and WAIT COND): ({hex(offset)} & {hex(mask)}) != {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TRUE")

        elif cmd in [-18, -26]:
            offset, mask, expected, delay, pos = struct.unpack("<LLLLl", f.read(0x14))
            pos += 1
            
            instr[ip].append(f"&assert_i = 0")
            instr[ip].append(f"while &assert_i < {delay + 1}")
            instr[ip].append("{")
            if cmd == -18:
                instr[ip].append(f"    &assert_c = data.long(0x{offset:08x})")
                instr[ip].append(f"    if (&assert_c & 0x{mask:08x}) == 0x{expected:08x}")
                instr[ip].append(f"        goto has_{ip+pos}")
                instr[ip].append(f"    &assert_i = &assert_i + 1")
                labels[ip+pos] = f"has_{ip+pos}"
                
            elif cmd == -26:
                instr[ip].append(f"    &assert_c = data.word(0x{offset:08x})")
                instr[ip].append(f"    if (&assert_c & 0x{mask:04x}) == 0x{expected:04x}")    
                instr[ip].append(f"        goto has_{ip+pos}")
                instr[ip].append(f"    &assert_i = &assert_i + 1")
                labels[ip+pos] = f"has_{ip+pos}"
            
            instr[ip].append("}")
            #print(f"{cmd} (READ and WAIT COND): ({hex(offset)} & {hex(mask)}) == {expected}, max: {delay}ms, SKIP {branch} INSTRUCTION if TRUE")

        elif cmd == -54:
            offset, bits = struct.unpack("<LL", f.read(0x8))
            instr[ip].append(f"// {cmd} (READ BYTES and PRINT) {hex(offset)} {bits}")

        elif cmd == -59:
            offset, count, value = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (WRITE8) {hex(offset)} {hex(value)} {count}")

        elif cmd == -58:
            offset, count, value = struct.unpack("<LLL", f.read(0xc))
            print(f"{cmd} (WRITE16) {hex(offset)} {hex(value)} {count}")

        elif cmd == -41:
            reg = int.from_bytes(f.read(4), "little")
            instr[ip].append(f'print "0x{reg:02x}"')
            #print(f"{cmd} (PRINT): {hex(reg)}")

        elif cmd == -38:
            mask, cond, pos = struct.unpack("<LLl", f.read(0xc))
            pos += 1
            
            instr[ip].append(f"if (&var_a & 0x{mask:08x}) == 0x{expected:08x}")
            instr[ip].append(f"    goto has_{ip+pos}")
            labels[ip+pos] = f"has_{ip+pos}"
            #print(f"{cmd} (COND): (a & {hex(mask)}) == {hex(cond)}, SKIP {branch} INSTRUCTION if TRUE")

        elif cmd == -39:
            mask, cond, pos = struct.unpack("<LLl", f.read(0xc))
            pos += 1
            
            instr[ip].append(f"if (&var_a & 0x{mask:08x}) != 0x{expected:08x}")
            instr[ip].append(f"    goto has_{ip+pos}")
            labels[ip+pos] = f"has_{ip+pos}"
            #print(f"{cmd} (COND): (a & {hex(mask)}) != {hex(cond)}, SKIP {branch} INSTRUCTION if TRUE")

        elif cmd == -255:
            instr[ip].append("end")
            #print(f"{cmd} (RETURN)")
            
        elif cmd == -16:
            instr[ip].append(f"// {cmd}: {f.read(4)}")
            
        elif cmd == -65536:
            instr[ip].append(f"// {cmd}: {f.read(8)}")

        else:
            raise Exception(f"command {cmd} {hex(f.tell() - 4)}")
        
        ip += 1
        
    ip = 0
    while ip in instr:
        if ip in labels: print(f"{labels[ip]}:")
        for r in instr[ip]:
            print(r)
        ip += 1