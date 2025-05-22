#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    WRITE_U32(0x53000000, 0x0);

    WRITE_U32(0x4a000008, 0xffffffff);
    WRITE_U32(0x4a00001c, 0x000007ff);
    WRITE_U32(0x4a000000, 0xffffffff);
    WRITE_U32(0x4a000018, 0x000007ff);
    WRITE_U32(0x4a000010, 0xffffffff);
}

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
}