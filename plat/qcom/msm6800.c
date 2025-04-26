#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x80005410, 1);
}

void wdog_reset(void) {
    WRITE_U32(0x8000540c, 1);
    // Reset watchdog (or else, system restarts)
}