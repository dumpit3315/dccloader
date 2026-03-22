#include <stdint.h>

typedef struct {
    uint32_t offset;
    uint32_t type;
} RELOC;

void pic_relocate(uint32_t base, RELOC *reloc_cur, RELOC *reloc_end, uint32_t data_end_offset) {
    while (reloc_cur < reloc_end) {
        RELOC *rel = reloc_cur++;
        if (rel->type == 0x17 && rel->offset >= data_end_offset) 
            *((uint32_t *)(base + rel->offset)) += base;
    }
}