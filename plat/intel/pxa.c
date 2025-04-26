#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    WRITE_U16(0x40A00018, 0x0); // Shut up watchdog
    WRITE_U16(0x40A0001C, 0x0); // Shut up interrupts
}

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
}