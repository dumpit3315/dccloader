#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    WRITE_U32(0x30503000, 0x0);  // Shut up watchdog
}

#define DOG_TIMEOUT 0x3fff

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
    // WRITE_U16(0x30503004, 0x8000 | DOG_TIMEOUT);
}