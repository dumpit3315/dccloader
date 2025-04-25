/* Nand controller template */
#include "controller.h"

void inline NAND_Ctrl_Command_Write(uint8_t cmd) {
    // Write command routines
    wdog_reset();
}

void inline NAND_Ctrl_Address_Write(uint8_t addr) {
    // Write address routines
    wdog_reset();
}

uint16_t inline NAND_Ctrl_Data_Read() {
    // Data read routines
    wdog_reset();
    return 0;
}

void inline NAND_Ctrl_Wait_Ready() {
    // Busy assert routines
    wdog_reset();
}

uint32_t inline NAND_Ctrl_Check_Status() {
    return 1;
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;

    NAND_Ctrl_Command_Write(NAND_CMD_RESET);
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

    NAND_Ctrl_Command_Write(NAND_CMD_RESET);
    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    
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

    return DCC_OK;
}