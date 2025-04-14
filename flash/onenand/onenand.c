#include "onenand.h"
#include "dcc/dn_dcc_proto.h"
#include "controller/controller.h"

void OneNAND_Ctrl_Wait_Ready(DCCMemory *mem, uint16_t flag) {
    // Busy assert routines
    do {
        wdog_reset();
    } while ((OneNAND_Ctrl_Reg_Read(mem, O1N_REG_INTERRUPT) & flag) != flag);
}

uint32_t OneNAND_Probe(DCCMemory *mem, uint32_t offset) {
    wdog_reset();
    OneNAND_Pre_Initialize(mem, offset);
    
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_SYS_CFG1, 0x40c0);

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS1, 0x0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS2, 0x0);

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_INTERRUPT, 0x0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_COMMAND, O1N_CMD_HOT_RESET);

    OneNAND_Ctrl_Wait_Ready(mem, 0x8000);

    uint16_t mfr_id = OneNAND_Ctrl_Reg_Read(mem, O1N_REG_MANUFACTURER_ID);
    uint16_t dev_id = OneNAND_Ctrl_Reg_Read(mem, O1N_REG_DEVICE_ID);

    mem->device_id = dev_id;
    mem->manufacturer = mfr_id;
    mem->type = MEMTYPE_ONENAND;

    mem->bit_width = 16;
    uint32_t density = 2 << ((mem->page_size == 4096 ? 4 : 3) + ((dev_id >> 4) & 0xf));

    mem->size = density << 20;
    mem->block_size = mem->page_size * 0x40;

    return DCC_OK;
}

uint32_t OneNAND_Read_Upper(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_INTERRUPT, 0x0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_ECC_STATUS, 0x0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_BUFFER, (8 << 8)); // First DataRAM

    uint32_t density = 2 << ((mem->page_size == 4096 ? 4 : 3) + ((mem->device_id >> 4) & 0xf));        
    uint32_t addr1_mask = ((mem->device_id & 8) ? (density << 2) : (density << 3)) - 1;
    uint32_t ddp_access = (mem->device_id & 8) && ((page >> 6) >= (density << 2));

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS1, (ddp_access ? 0x8000 : 0) | ((page >> 6) & addr1_mask));
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS2, ddp_access ? 0x8000 : 0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_START_ADDRESS8, (page & 63) << 2);

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_COMMAND, O1N_CMD_READ);
    OneNAND_Ctrl_Wait_Ready(mem, 0x8080);

    OneNAND_Ctrl_Get_Data(mem, page_buf, spare_buf, mem->page_size, mem->page_size / 0x20);
    return DCC_OK;
}

DCC_RETURN OneNAND_Read(DCCMemory *mem, uint32_t offset, uint32_t size, uint8_t *dest, uint32_t *dest_size) {
    uint32_t page_offset = 0;
    uint32_t spare_offset = size;
    DCC_RETURN ret_code;

    if (size % mem->page_size) return DCC_INVALID_ARGS;
    
    do {
        ret_code = OneNAND_Read_Upper(mem, dest + page_offset, dest + spare_offset, offset >> DN_Log2(mem->page_size));
        if (ret_code != DCC_OK) return ret_code;
        offset += mem->page_size;
        page_offset += mem->page_size;
        spare_offset += mem->page_size / 0x20;
        size -= mem->page_size;
    } while (size);

    *dest_size = spare_offset;
    return DCC_OK;
}

Driver onenand_controller = {
    .initialize = OneNAND_Probe,
    .read = OneNAND_Read
};