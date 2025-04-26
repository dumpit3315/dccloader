#!/usr/bin/env python
# Sample code for ARM of Unicorn. Nguyen Anh Quynh <aquynh@gmail.com>
# Python sample ported by Loi Anh Tuan <loianhtuan@gmail.com>

from __future__ import print_function
from unicorn import *
from unicorn.arm_const import *
import time
import struct

DEBUG = True

bp_offset = 0
WAIT_RESPONSE = False
data_rd = b""

# callback for tracing basic blocks
def hook_block(uc, address, size, user_data):
    pass
    #print(">>> Tracing basic block at 0x%x, block size = 0x%x" %(address, size))


# callback for tracing instructions
def hook_code(uc: Uc, address, size, user_data):    
    global status_reg, wr_reg, WAIT_RESPONSE, data_rd
    
    try:
        #if address == 0x14001da0: uc.mem_write(0x0, b"\x01\x00\x7e\x22")
        if DEBUG:
            print(">>> Tracing instruction at 0x%x, instruction size = 0x%x" %(address, size))
            print("CODE:",uc.mem_read(address, size))
            print("RSP1", hex(uc.reg_read(UC_ARM_REG_R0)))
            print("RSP2", hex(uc.reg_read(UC_ARM_REG_R1)))
            print("RSP3", hex(uc.reg_read(UC_ARM_REG_R2)))
            print("RSP4", hex(uc.reg_read(UC_ARM_REG_R3)))
            print("SP", hex(uc.reg_read(UC_ARM_REG_SP)))
            print("SP_DATA", uc.mem_read(uc.reg_read(UC_ARM_REG_SP), 0x10)) 
        
        if address == 0x14000000 + bp_offset:
            WAIT_RESPONSE = True
            
            m_size, m_start = struct.unpack("<LL", uc.mem_read(0x1400002c, 8))
            data_rd = bytes(uc.mem_read(m_start, m_size))
            
            while WAIT_RESPONSE:
                time.sleep(1)

            uc.mem_write(m_start, data_rd)
            uc.reg_write(UC_ARM_REG_PC, address + 4)
            return
        
    except KeyboardInterrupt:
        uc.emu_stop()
        raise
    # cp = 15
    # is64 = 0
    # sec = 0
    # crn = 1
    # crm = 0
    # opc1 = 0
    # opc2 = 0
    # val = ??
   
# Test ARM
def test_arm():
    global bp_offset
    
    print("Emulate ARM code")
    try:
        # Initialize emulator in ARM mode
        mu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
        mu.ctl_exits_enabled(True)
        mu.ctl_set_exits([0])

        mu.mem_map(0x00000000, 32 * 1024 * 1024)
        mu.mem_map(0x03000000, 2 * 1024 * 1024)

        # map 2MB memory for this emulation
        mu.mem_map(0x14000000, 2 * 1024 * 1024)

        # write machine code to be emulated to memory
        mu.mem_write(0x14000000, open("build/dumpnow.bin", "rb").read())       
        bp_offset = int.from_bytes(mu.mem_read(0x14000034, 4), "little")
        #mu.mem_write(0x00000000, open("cfi_32mb.bin", "rb").read()) 
        #mu.mem_write(0x00000000, b"\x01\x00\x7e\x22") # Infineon NOR
        #mu.mem_write(0x14000020, b"\x00\x00\x00\x00") # Infineon NOR
        #mu.mem_write(0x00000020, b"Q\0R\0Y\0\x02\0\0\0")
        #mu.mem_write(0x0000004e, (23).to_bytes(2, "little"))

        #mu.mem_write(0x14000020, b"\0\0\0\x10")        

        # initialize machine registers
        mu.reg_write(UC_ARM_REG_APSR, 0xFFFFFFFF) #All application flags turned on        
   
        # tracing all basic blocks with customized callback
        mu.hook_add(UC_HOOK_BLOCK, hook_block)

        # tracing one instruction at ADDRESS with customized callback
        mu.hook_add(UC_HOOK_CODE, hook_code)
        
        def on_read(mu, access, address, size, value, data):
            #if DEBUG and address <= 0x14000000:
            if DEBUG:
                print("Read at", hex(address), size, mu.mem_read(address, size))

        def on_write(mu, access, address, size, value, data):
            if DEBUG:
                if address <= 0x14000000:
                    if address == 0xaaa and value == 0x98:
                        mu.mem_write(0x00000000, open("cfi_32mb.bin", "rb").read())
                        
                    elif address == 0xaaa and value == 0x90:
                        mu.mem_write(0x00000000, b"\x01\x00\x7e\x22")
                        
                    elif address == 0x0 and value == 0xf0:
                        mu.mem_write(0x00000000, open("build/dumpnow.bin", "rb").read())
                # mu.reg_write(0x)
                print("Write at", hex(address), size, hex(value))
                # if value == 0x98:
                #     mu.mem_write(0x0, open("cfi.bin", "rb").read())
                    
                # elif value == 0xf0:
                #     mu.mem_write(0x0, open("RIFF_Nor_DCC_Test.bin", "rb").read())
                    
                # else:
                #     mu.mem_write(address, value.to_bytes(size, "little"))

        def on_error(mu, access, address, size, value, data):
            if DEBUG:
                print("Error at", hex(address), size, hex(value), "in", hex(mu.reg_read(UC_ARM_REG_PC)), "lr", hex(mu.reg_read(UC_ARM_REG_LR)))        

        mu.hook_add(UC_HOOK_MEM_READ, on_read)
        mu.hook_add(UC_HOOK_MEM_WRITE, on_write)
        mu.hook_add(UC_HOOK_MEM_INVALID, on_error)

        # emulate machine code in infinite time
        mu.emu_start(0x14000000, 0x1440000)

        # now print out some registers
        print(">>> Emulation done. Below is the CPU context")

        r0 = mu.reg_read(UC_ARM_REG_R0)
        r1 = mu.reg_read(UC_ARM_REG_R1)
        print(">>> R0 = 0x%x" %r0)
        print(">>> R1 = 0x%x" %r1)                

    except UcError as e:
        print("ERROR: %s" % e)

def _dcc_read_host():
    global data_rd
    while not WAIT_RESPONSE:
        time.sleep(0.1)
            
    temp = data_rd[:4]
    data_rd = data_rd[4:]
    
    return int.from_bytes(temp, "little")
    
def _dcc_write_host(data):
    global data_rd
    assert WAIT_RESPONSE, "cannot do that while running!"
    data_rd += data.to_bytes(4, "little")

if __name__ == '__main__':
    import threading
    import time
    
    t = threading.Thread(target=test_arm, daemon=True)
    t.start()
    
    iCount = _dcc_read_host()
    print("C:", hex(iCount))
    
    for _ in range(iCount + 1):
        print("H:", hex(_dcc_read_host()))
        
    data_rd = b""
    print("RUN")
    _dcc_write_host(0x152 | 0x00000000)
    _dcc_write_host(0x00120000)
    _dcc_write_host(0x00000080)
    
    WAIT_RESPONSE = False
    while len(data_rd) > 0:
        print("H:", hex(_dcc_read_host()))
    
    # while True:
    #     time.sleep(2)