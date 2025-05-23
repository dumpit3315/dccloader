#include <stdint.h>

extern void dcc_main(uint32_t StartAddress, uint32_t PageSize);

int main(void) {
    dcc_main(0x0, 0x800);
    return 0;
}