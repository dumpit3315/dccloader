#include <stdint.h>

typedef struct {
    uint32_t offset;
    uint32_t type;
} RELOC;

void pic_relocate(uint32_t base, uint32_t reloc_cur, uint32_t reloc_end, uint32_t data_offs) {
    while (reloc_cur < reloc_end) {
        RELOC *rel = (RELOC *)(reloc_cur);
        if (rel->type == 0x17 && rel->offset >= data_offs) rel->offset += base;
        reloc_cur += sizeof(RELOC);
    }
}