#include "superand.h"
#include "controller/controller.h"
#include "dcc/dn_dcc_proto.h"

DCC_RETURN SuperAND_Probe(DCCMemory *mem, uint32_t offset) {
    return SuperAND_Ctrl_Probe(mem);
}

DCC_RETURN SuperAND_Read(DCCMemory *mem, uint32_t offset, uint32_t size, uint8_t *dest, uint32_t *dest_size) {
    uint32_t page_offset = 0;
    DCC_RETURN ret_code;

    if (size & (mem->page_size - 1)) return DCC_INVALID_ARGS;
    
    do {
        ret_code = SuperAND_Ctrl_Read(mem, dest + page_offset, offset >> DN_Log2(mem->page_size));
        if (ret_code != DCC_OK) return ret_code;

        offset += mem->page_size;
        page_offset += mem->page_size;
        size -= mem->page_size;
    } while (size);

    *dest_size = page_offset;
    return DCC_OK;
}

Driver superand_controller = {
    .initialize = SuperAND_Probe,
    .read = SuperAND_Read
};