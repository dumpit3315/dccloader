/* OneNAND controller template */

#include "../onenand.h"
#include "controller.h"
#include "stdint.h"
#include "memory.h"
#include "dcc/dn_dcc_proto.h"

void OneNAND_Pre_Initialize(DCCMemory *mem, uint32_t offset) {
    // Initialize routines
    mem->base_offset = offset;
    mem->page_size = 0x800;
}

void OneNAND_Ctrl_Reg_Write(DCCMemory *mem, uint16_t reg, uint16_t data) {
    // Write register routines
    WRITE_U16(mem->base_offset + (reg << 1), data);
}

uint16_t OneNAND_Ctrl_Reg_Read(DCCMemory *mem, uint16_t reg) {
    // Read register routines
    return READ_U16(mem->base_offset + (reg << 1));
}

void OneNAND_Ctrl_Get_Data(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page_size, uint32_t spare_size) {
    memcpy(page_buf, (uint8_t *)(mem->base_offset + (O1N_DATARAM << 1)), page_size);
    memcpy(spare_buf, (uint8_t *)(mem->base_offset + (O1N_SPARERAM << 1)), spare_size);
}

