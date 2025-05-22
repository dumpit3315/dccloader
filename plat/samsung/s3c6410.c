#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    WRITE_U32(0x7e004000, 0x0);

    WRITE_U32(0x71200014, 0xffffffff);
    WRITE_U32(0x71300014, 0xffffffff);

    WRITE_U32(0x7120000c, 0x0);
    WRITE_U32(0x7130000c, 0x0);

    WRITE_U32(0x71200f00, 0x0);
    WRITE_U32(0x71300f00, 0x0);
}

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
}