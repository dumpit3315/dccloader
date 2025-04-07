#include "dcc/dn_dcc_proto.h"
#include "mmap.h"

void Memdump_Probe(uint32_t offset, DCCMemory *mem) {    
    mem->manufacturer = MEMDUMP_MFR;
    mem->device_id = MEMDUMP_DEVID;
    mem->bit_width = 16;
    mem->size = MEMDUMP_MAX_SIZE;
    mem->type = MEMTYPE_NOR;
    mem->nor_cmd_set = 2;
}