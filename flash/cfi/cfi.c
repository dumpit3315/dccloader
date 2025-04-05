#include "dcc/dn_dcc_proto.h"
#include "cfi.h"

#define CFI_READ(o, x) READ_U16(o + (x * 2))
#define CFI_WRITE(o, x, y) WRITE_U16(o + (x * 2), y)

typedef struct {
    uint8_t bit_width;
    uint32_t size;
} CFIQuery;

uint32_t CFI_Query(uint32_t offset, uint32_t type, CFIQuery *qry) {
    /* 01 - Check if CFI is supported*/
    if (type == 2) {
        CFI_WRITE(offset, 0x555, 0x98); // S29WS-N/S29PL-N
        if ((CFI_READ(offset, 0x10) != 0x51) || (CFI_READ(offset, 0x11) != 0x52) || (CFI_READ(offset, 0x12) != 0x59)) {
            CFI_WRITE(offset, 0x55, 0x98); // S29GL-N
        }
    } else {
        CFI_WRITE(offset, 0, 0x98);
    }

    /* Early Sharp LRS/Renesas M6M doesn't have CFI, so bail that while we find a suitable size using ID. */
    if ((CFI_READ(offset, 0x10) != 0x51) || (CFI_READ(offset, 0x11) != 0x52) || (CFI_READ(offset, 0x12) != 0x59)) return 0;
    if (CFI_READ(offset, 0x13) != type) return 0; // Check for valid types

    qry->bit_width = 16;
    qry->size = 1 << CFI_READ(offset, 0x27);

    /* Alternatively, use erase regions for size computing */    
    /*
    uint16_t qry_erase_size = CFI_READ(offset, 0x2c);
    for (uint16_t i = 0; i < qry_erase_size; i++) {
        uint16_t y = CFI_READ(offset, 0x2d + (4 * i)) | (CFI_READ(offset, 0x2e + (4 * i)) << 8);
        uint16_t z = CFI_READ(offset, 0x2f + (4 * i)) | (CFI_READ(offset, 0x30 + (4 * i)) << 8);
        qry->size += (y + 1) * (z << 8);
    }
    */

    return 1;
}

void CFI_Probe(uint32_t offset, DCCMemory *mem) {    
    uint32_t CFI_Type;
    CFIQuery qry = { 0 };
    
    // 01 - CFI Query
    for (CFI_Type = 1; CFI_Type < 4; CFI_Type++) {
        CFI_WRITE(offset, 0, 0xff);
        CFI_WRITE(offset, 0, 0xf0);

        if (CFI_Query(offset, CFI_Type, &qry)) break;
    }

    // If we can't find support for CFI, use predefined values. (Renesas/Early Sharp LRS)
    if (CFI_Type == 4) {
        qry.bit_width = 16;
        qry.size = 0x02000000;
    }

    // 02 - Get Manufacturer
    if (CFI_Type == 2) { // Spansion
        CFI_WRITE(offset, 0x555, 0xaa);
        CFI_WRITE(offset, 0x2aa, 0x55);
        CFI_WRITE(offset, 0x555, 0x90);
    } else { // Something else
        CFI_WRITE(offset, 0x00, 0x90);
    }

    mem->manufacturer = (uint8_t)CFI_READ(offset, 0x00);
    mem->device_id = CFI_READ(offset, 0x01);
    mem->bit_width = qry.bit_width;
    mem->size = qry.size;
    mem->type = MEMTYPE_NOR;
    mem->nor_cmd_set = CFI_Type;
}