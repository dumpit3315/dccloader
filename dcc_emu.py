#!/usr/bin/env python
# Sample code for ARM of Unicorn. Nguyen Anh Quynh <aquynh@gmail.com>
# Python sample ported by Loi Anh Tuan <loianhtuan@gmail.com>

from __future__ import print_function
from unicorn import *
from unicorn.arm_const import *
from capstone import *
from capstone.arm import *

DEBUG = True

# callback for tracing basic blocks
def hook_block(uc, address, size, user_data):
    pass
    #print(">>> Tracing basic block at 0x%x, block size = 0x%x" %(address, size))

status_reg = 0b0100 << 28 # By default, the DCC loader can write, but not read
rd_reg = 0
wr_reg = 0

# callback for tracing instructions
def hook_code(uc: Uc, address, size, user_data):    
    global status_reg, wr_reg
    
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
        
        cs = Cs(CS_ARCH_ARM, CS_MODE_THUMB if uc.reg_read(UC_ARM_REG_CPSR) & (1 << 5) else CS_MODE_ARM)
        cs.detail = True
        
        ins: CsInsn = [x for x in cs.disasm(uc.mem_read(address, size), address)][0]
        if ins.id == ARM_INS_MRC:        
            if ins.operands[0].value.imm == 14:                        
                opc_1 = ins.operands[1].value.imm
                cp_dest = ins.operands[2].value.reg
                cr_n = ins.operands[3].value.imm
                cr_m = ins.operands[4].value.imm
                opc_2 = ins.operands[5].value.imm
                
                if cr_n == 0:                
                    uc.reg_write(cp_dest, status_reg)
                    
                elif cr_n == 1:                
                    print("DCC HOST -> OCD", hex(rd_reg))
                    uc.reg_write(cp_dest, rd_reg)
                    status_reg &= ~1 # Sets the R bit to low, indicating that the host has finished processing the data.                                            
                    
                uc.reg_write(UC_ARM_REG_PC, address+size) # Skip this instruction as we've already processed some DCC logic
            
        elif ins.id == ARM_INS_MCR:        
            if ins.operands[0].value.imm == 14:              
                opc_1 = ins.operands[1].value.imm
                cp_dest = ins.operands[2].value.reg
                cr_n = ins.operands[3].value.imm
                cr_m = ins.operands[4].value.imm
                opc_2 = ins.operands[5].value.imm
                
                if cr_n == 0:
                    status_reg = uc.reg_read(cp_dest)
                    
                elif cr_n == 1:                
                    wr_reg = uc.reg_read(cp_dest)                
                    print("DCC OCD -> HOST", hex(wr_reg))
                    status_reg |= 2 # Sets the W bit to high, indicating that the debugger is ready to process the data.
                    
                uc.reg_write(UC_ARM_REG_PC, address+size) # Skip this instruction as we've already processed some DCC logic
        
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
        #mu.mem_write(0x00000000, open("cfi_32mb.bin", "rb").read()) 
        #mu.mem_write(0x00000000, b"\x01\x00\x7e\x22") # Infineon NOR
        mu.mem_write(0x14000020, b"\x00\x00\x00\x00") # Infineon NOR
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
    global status_reg
    if (status_reg & 2) == 0: return 0
    
    print("DBG READ")
    temp = wr_reg
    
    status_reg &= ~2 # Debugger finally processed the data and set the W bit to low. 
    return temp
    
def _dcc_write_host(data):
    import time
    global status_reg, rd_reg
    
    while _dcc_read_status_host() & 1: time.sleep(0.1)
    
    print("DBG WRTIE")
    rd_reg = data
    status_reg |= 1 # With the R bit set to high, the host was as motivated to process the data.            
    
def _dcc_read_status_host():
    return status_reg

if __name__ == '__main__':
    import threading
    import time
    
    t = threading.Thread(target=test_arm, daemon=True)
    t.start()
    
    while (_dcc_read_status_host() & 2) == 0: time.sleep(0.1)
    iCount = _dcc_read_host()
    print("C:", hex(iCount))
    
    for _ in range(iCount + 1):
        while (_dcc_read_status_host() & 2) == 0: time.sleep(0.1)
        print("H:", hex(_dcc_read_host()))
        
    print("RUN")
    _dcc_write_host(0x152 | 0x00000000)
    _dcc_write_host(0x00120000)
    _dcc_write_host(0x00000080)
    
    while True:
        while (_dcc_read_status_host() & 2) == 0: time.sleep(0.1)
        print("H:", hex(_dcc_read_host()))
    
    while True:
        time.sleep(2)