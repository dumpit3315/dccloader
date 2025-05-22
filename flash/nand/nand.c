#include "nand.h"
#include "controller/controller.h"
#include "dcc/dn_dcc_proto.h"

DCC_RETURN NAND_Probe(DCCMemory *mem, uint32_t offset) {
    return NAND_Ctrl_Probe(mem);
}

DCC_RETURN NAND_Read(DCCMemory *mem, uint32_t offset, uint32_t size, uint8_t *dest, uint32_t *dest_size) {
    uint32_t page_offset = 0;
    uint32_t spare_offset = size;
    DCC_RETURN ret_code;

    if (size & (mem->page_size - 1)) return DCC_INVALID_ARGS;
    
    do {
        ret_code = NAND_Ctrl_Read(mem, dest + page_offset, dest + spare_offset, offset >> DN_Log2(mem->page_size));
        if (ret_code != DCC_OK) return ret_code;
        offset += mem->page_size;
        page_offset += mem->page_size;
        spare_offset += mem->page_size >> 5;
        size -= mem->page_size;
    } while (size);

    *dest_size = spare_offset;
    return DCC_OK;
}

Driver nand_controller = {
    .initialize = NAND_Probe,
    .read = NAND_Read
};