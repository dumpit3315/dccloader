/* Nand controller template */
#include "controller.h"

void SuperAND_Ctrl_Command_Write(uint8_t cmd) {
    // Write command routines
    wdog_reset();
}

void SuperAND_Ctrl_Address_Write(uint8_t addr) {
    // Write address routines
    wdog_reset();
}

uint16_t SuperAND_Ctrl_Data_Read() {
    // Data read routines
    wdog_reset();
    return 0;
}

void SuperAND_Ctrl_Wait_Ready() {
    // Busy assert routines
    wdog_reset();
}

uint32_t SuperAND_Ctrl_Check_Status() {
    return 1;
}

DCC_RETURN SuperAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;

    SuperAND_Ctrl_Command_Write(SUPERAND_CMD_READID);
    SuperAND_Ctrl_Address_Write(0x0);

    uint16_t mfr_id = SuperAND_Ctrl_Data_Read();
    uint16_t dev_id = SuperAND_Ctrl_Data_Read();

    for (int i = 0; flash_ids[i].dev_id; i++) {
        if (dev_id == (uint16_t)flash_ids[i].dev_id) {
            mem->device_id = dev_id;
            mem->manufacturer = mfr_id;
            mem->bit_width = flash_ids[i].bits;
            mem->block_size = flash_ids[i].block_size;
            mem->page_size = flash_ids[i].page_size;
            mem->size = flash_ids[i].chip_size;
            mem->type = MEMTYPE_SUPERAND;
            break;
        }
    }

    if (mem->type != MEMTYPE_SUPERAND) return DCC_PROBE_ERROR;
    return DCC_OK;
}

DCC_RETURN SuperAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint32_t page) {
    wdog_reset();
    
    SuperAND_Ctrl_Command_Write(SUPERAND_CMD_READ);

    uint32_t sand_page = page & 3;
    uint32_t sand_sector = page >> 2;

    SuperAND_Ctrl_Address_Write(0);
    SuperAND_Ctrl_Address_Write(mem->bit_width == 8 ? (sand_page << 1) : sand_page);
    SuperAND_Ctrl_Address_Write(sand_sector);
    SuperAND_Ctrl_Address_Write(sand_sector >> 8);

    SuperAND_Ctrl_Wait_Ready();
    if (!SuperAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

    for (int i = 0; i < (mem->page_size >> (mem->bit_width >> 4)); i++) {
        wdog_reset();
        if (mem->bit_width == 16) {
            ((uint16_t *)(page_buf))[i] = SuperAND_Ctrl_Data_Read();
        } else {
            page_buf[i] = (uint8_t)SuperAND_Ctrl_Data_Read();
        }
    }

    SuperAND_Ctrl_Command_Write(SUPERAND_CMD_READ_EXIT);
    return DCC_OK;
}