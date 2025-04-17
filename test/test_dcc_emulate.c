#include <stdint.h>
#include <stdio.h>

extern void dcc_main(uint32_t BaseAddress1, uint32_t BaseAddress2, uint32_t BaseAddress3, uint32_t BaseAddress4);

int main(void) {
    dcc_main(0x0, 0x0, 0x0, 0x0);
    return 0;
}