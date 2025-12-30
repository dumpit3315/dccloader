/* OneNAND controller template */

#include "../onenand.h"
#include "controller.h"
#include <stdint.h>
#include "dcc/dn_dcc_proto.h"

void OneNAND_Pre_Initialize(DCCMemory *mem, uint32_t offset) {
    // Initialize routines
    mem->base_offset = offset;
    mem->page_size = 0x800;

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_SYS_CFG1, 0x40c0, 0);

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS1, 0x0, 0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS2, 0x0, 0);

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_INTERRUPT, 0x0, 0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_COMMAND, O1N_CMD_HOT_RESET, 1);
}

int OneNAND_Ctrl_Wait_Ready(DCCMemory *mem, uint16_t flag) {
    // Busy assert routines
    int timeout = 0x10000;

    do {
        wdog_reset();
        if (timeout == 0) return 0;
        timeout--;
    } while ((OneNAND_Ctrl_Reg_Read(mem, O1N_REG_INTERRUPT) & flag) != flag);

    return 1;
}

void OneNAND_Ctrl_Reg_Write(DCCMemory *mem, uint16_t reg, uint16_t data, uint8_t wait_interrupt) {
    // Write register routines
    WRITE_U16(mem->base_offset + (reg << 1), data);
}

uint16_t OneNAND_Ctrl_Reg_Read(DCCMemory *mem, uint16_t reg) {
    // Read register routines
    return READ_U16(mem->base_offset + (reg << 1));
}

void OneNAND_Ctrl_Reg_Write_Queue(DCCMemory *mem, uint16_t reg, uint16_t data) {
    OneNAND_Ctrl_Reg_Write(mem, reg, data, 0);
}

int OneNAND_Ctrl_Execute_Queue(uint8_t wait_interrupt) {
    return 1;
}

void OneNAND_Ctrl_Get_Data(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page_size, uint32_t spare_size) {
    PLAT_MEMCPY(page_buf, (uint8_t *)(mem->base_offset + (O1N_DATARAM << 1)), page_size);
    PLAT_MEMCPY(spare_buf, (uint8_t *)(mem->base_offset + (O1N_SPARERAM << 1)), spare_size);
}

