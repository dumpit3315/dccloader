// Broadcomm BCM21xxx NAND Controller
// Found in Samsung GT-S7070
#include "../controller.h"
#include "bcm2133.h"

static uint8_t is_axi;
static uint8_t axi_cmd;
static uint8_t bit_width;

void inline NAND_Ctrl_Command_Write(uint8_t cmd) {
    // Write command routines
    wdog_reset();
    if (is_axi) {
        axi_cmd = cmd;
        if (cmd == NAND_CMD_RESET) WRITE_U32(BCM_AXI_CMD_RESET, 0);
    } else {
        WRITE_U16(BCM_NAND_CMD, cmd);
    }
}

void inline NAND_Ctrl_Address_Write(uint8_t addr) {
    // Write address routines
    wdog_reset();
    if (is_axi) {
        if (axi_cmd == NAND_CMD_READID) WRITE_U32(BCM_AXI_CMD_READID, addr);
    } else {
        WRITE_U16(BCM_NAND_ADDR, addr);
    }
}

uint16_t inline NAND_Ctrl_Data_Read() {
    // Data read routines
    wdog_reset();
    if (is_axi) {
        if (axi_cmd == NAND_CMD_READID) {
            return bit_width == 16 ? READ_U16(BCM_AXI_READ_ID) : READ_U8(BCM_AXI_READ_ID);
        }
        return bit_width == 16 ? READ_U16(BCM_AXI_READ_BUF) : READ_U8(BCM_AXI_READ_BUF);
    } else {
        return bit_width == 16 ? READ_U16(BCM_NAND_DATA) : READ_U8(BCM_NAND_DATA);
    }
}

void inline NAND_Ctrl_Wait_Ready() {
    // Busy assert routines
    wdog_reset();
    if (is_axi) {
        int wait_count = 0x40;
        do { 
            if (!(READ_U8(BCM_AXI_LOCK) & 2)) break;
            wdog_reset(); 
        } while (wait_count--);
        do { wdog_reset(); } while (!(READ_U8(BCM_AXI_LOCK) & 2));
    } else {
        int wait_count = 0x40;
        do { 
            if (!(READ_U32(BCM_NAND_STATUS) & NAND_STATUS_READY)) break;
            wdog_reset(); 
        } while (wait_count--);
        do { wdog_reset(); } while (!(READ_U32(BCM_NAND_STATUS) & NAND_STATUS_READY));
    }
}

uint32_t inline NAND_Ctrl_Check_Status() {
    return 1;
}

uint32_t inline IS_16BIT(uint32_t axi_flag) {
    uint32_t temp = READ_U32(BCM_AXI_BIT);

    if (axi_flag != 0x90 && axi_flag != 0xb0) {
        return ((temp >> 0x19) & 1) + 1;
    } else {
        return temp & 2;
    }
}

void inline NANDC_Lock(uint8_t lock) {
    if (lock) {
        WRITE_U8(BCM_AXI_LOCK, READ_U8(BCM_AXI_LOCK) | 1);
    } else {
        WRITE_U8(BCM_AXI_LOCK, READ_U8(BCM_AXI_LOCK) & ~1);
    }
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;

    uint32_t axi_flags = READ_U32(BCM_AXI_FLAGS) & 0xf0;
    is_axi = axi_flags == 0xf0 || axi_flags == 0xe0;
    bit_width = IS_16BIT(axi_flags) ? 16 : 8;

    if (is_axi) {
        WRITE_U32(0x088800dc, 0x10FF440D);
        WRITE_U32(0x08090018, bit_width == 16);
        WRITE_U32(0x08090014, 0x00092ae7);
        WRITE_U32(0x08090010, 0x2c00000);
        WRITE_U32(0x0809000c, 0x10);
    } else {
        WRITE_U32(0x08090018, READ_U32(0x08090018) | 0x80000000);
        NANDC_Lock(1);

        if (bit_width == 16) {
            WRITE_U32(0x08090018, READ_U32(0x08090018) | 0x40000000);
        } else {
            WRITE_U32(0x08090018, READ_U32(0x08090018) & ~0x40000000);
        }

        WRITE_U32(0x0809000c, READ_U32(0x0809000c) | 0x8000);
    }

    NAND_Ctrl_Command_Write(NAND_CMD_RESET);
    if (is_axi) {
        WRITE_U32(0x088ce004, READ_U32(0x088ce004) | 0x80000000);
        WRITE_U32(0x088ce004, READ_U32(0x088ce004) & ~0x40000000);
        WRITE_U32(0x088ce044, READ_U32(0x088ce044) & ~0xc0000000);
        WRITE_U32(0x088ce010, READ_U32(0x088ce010) | 0x80000000);
    }
    
    NAND_Ctrl_Command_Write(NAND_CMD_READID);
    NAND_Ctrl_Address_Write(0x0);

    uint16_t mfr_id = NAND_Ctrl_Data_Read();
    uint16_t dev_id = NAND_Ctrl_Data_Read();

    for (int i = 0; flash_ids[i].dev_id; i++) {
        if (dev_id == (uint16_t)flash_ids[i].dev_id) {
            mem->device_id = dev_id;
            mem->manufacturer = mfr_id;
            mem->bit_width = flash_ids[i].bits;
            mem->block_size = flash_ids[i].block_size;
            mem->page_size = flash_ids[i].page_size;
            mem->size = flash_ids[i].chip_size;
            mem->type = MEMTYPE_NAND;
            break;
        }
    }

    if (mem->type != MEMTYPE_NAND) return DCC_PROBE_ERROR;

    if (mem->page_size == 0) {
        NAND_Ctrl_Data_Read();
        uint8_t extra_id = (uint8_t)NAND_Ctrl_Data_Read();

        mem->page_size = 1 << (10 + (extra_id & 3));

        switch ((extra_id >> 4) & 3) {
            case 0:
                mem->block_size = 64 << 10;
                break;
            case 1:
                mem->block_size = 128 << 10;
                break;
            case 2:
                mem->block_size = 256 << 10;
                break;
            case 3:
                mem->block_size = 512 << 10;
                break;
        }

        mem->device_id |= extra_id << 8;
    }

    if (is_axi) {
        WRITE_U32(0x08090404, mem->page_size > 0x200 ? 0x17 : 0x15);
    }

    NANDC_Lock(0);

    if (is_axi) {
        WRITE_U32(0x088ce010, READ_U32(0x088ce010) & ~0x80000000);
    }

    NAND_Ctrl_Command_Write(NAND_CMD_RESET);
    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    
    if (is_axi) {
        if (mem->page_size > 0x200) {
            WRITE_U32(BCM_AXI_ADDR2, page << 16);
            WRITE_U32(BCM_AXI_ADDR2, page >> 16);
        } else {
            WRITE_U32(BCM_AXI_ADDR1, page << 8);
            WRITE_U32(BCM_AXI_ADDR2, page >> 24);
        }
        NAND_Ctrl_Wait_Ready();
        for (int i = 0; i < (mem->page_size >> (mem->bit_width >> 4)); i++) {
            wdog_reset();
            if (mem->bit_width == 16) {
                ((uint16_t *)(page_buf))[i] = NAND_Ctrl_Data_Read();
            } else {
                page_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }

        for (int i = 0; i < ((mem->page_size >> 5) >> (mem->bit_width >> 4)); i++) {
            wdog_reset();
            if (mem->bit_width == 16) {
                ((uint16_t *)(spare_buf))[i] = NAND_Ctrl_Data_Read();
            } else {
                spare_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }
    } else {
        NANDC_Lock(1);
        if (mem->page_size <= 512) {
            NAND_Ctrl_Command_Write(NAND_CMD_READ0);
            NAND_Ctrl_Address_Write(0);
            NAND_Ctrl_Address_Write(page);
            NAND_Ctrl_Address_Write(page >> 8);
            if (mem->size > 0x02000000) NAND_Ctrl_Address_Write(page >> 16);

            NAND_Ctrl_Wait_Ready();
            if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

            for (int i = 0; i < 0x100; i++) {
                wdog_reset();
                if (mem->bit_width == 16) {
                    ((uint16_t *)(page_buf))[i] = NAND_Ctrl_Data_Read();
                } else {
                    page_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
                }
            }

            if (mem->bit_width == 8) {
                NAND_Ctrl_Command_Write(NAND_CMD_READ1);
                NAND_Ctrl_Address_Write(0);
                NAND_Ctrl_Address_Write(page);
                NAND_Ctrl_Address_Write(page >> 8);
                if (mem->size > 0x02000000) NAND_Ctrl_Address_Write(page >> 16);
        
                NAND_Ctrl_Wait_Ready();
                if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;
        
                for (int i = 0; i < 0x100; i++) {
                    wdog_reset();
                    page_buf[i + 0x100] = (uint8_t)NAND_Ctrl_Data_Read();
                }
            }

            NAND_Ctrl_Command_Write(NAND_CMD_READOOB);
            NAND_Ctrl_Address_Write(0);
            NAND_Ctrl_Address_Write(page);
            NAND_Ctrl_Address_Write(page >> 8);
            if (mem->size > 0x02000000) NAND_Ctrl_Address_Write(page >> 16);

            NAND_Ctrl_Wait_Ready();
            if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

            for (int i = 0; i < (0x10 >> (mem->bit_width >> 4)); i++) {
                wdog_reset();
                if (mem->bit_width == 16) {
                    ((uint16_t *)(spare_buf))[i] = NAND_Ctrl_Data_Read();
                } else {
                    spare_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
                }
            }
        } else {
            NAND_Ctrl_Command_Write(NAND_CMD_READ0);
            NAND_Ctrl_Address_Write(0);
            NAND_Ctrl_Address_Write(0);
            NAND_Ctrl_Address_Write(page);
            NAND_Ctrl_Address_Write(page >> 8);
            if (mem->size > 0x08000000) NAND_Ctrl_Address_Write(page >> 16);

            NAND_Ctrl_Command_Write(NAND_CMD_READSTART);
            NAND_Ctrl_Wait_Ready();
            if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

            for (int i = 0; i < (mem->page_size >> (mem->bit_width >> 4)); i++) {
                wdog_reset();
                if (mem->bit_width == 16) {
                    ((uint16_t *)(page_buf))[i] = NAND_Ctrl_Data_Read();
                } else {
                    page_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
                }
            }

            for (int i = 0; i < ((mem->page_size >> 5) >> (mem->bit_width >> 4)); i++) {
                wdog_reset();
                if (mem->bit_width == 16) {
                    ((uint16_t *)(spare_buf))[i] = NAND_Ctrl_Data_Read();
                } else {
                    spare_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
                }
            }
        }
        NANDC_Lock(0);
    }

    return DCC_OK;
}