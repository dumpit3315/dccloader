// Memory dump interface

#include "mmap.h"
#include "dcc/dn_dcc_proto.h"

DCC_RETURN Memdump_Probe(DCCMemory *mem, uint32_t offset) {    
    mem->manufacturer = MEMDUMP_MFR;
    mem->device_id = MEMDUMP_DEVID;
    mem->bit_width = 16;
    mem->block_size = 0x10000;
    mem->page_size = 0x200;
    mem->size = MEMDUMP_MAX_SIZE;
    mem->type = MEMTYPE_NOR;
    mem->nor_cmd_set = 2;
    mem->base_offset = offset;

    return DCC_OK;
}

Driver memdump = {
    .initialize = Memdump_Probe,
    .read = NULL
};