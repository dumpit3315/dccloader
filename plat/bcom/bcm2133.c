#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    WRITE_U16(0x088a0000, 0x0); // Shut up watchdog
    WRITE_U16(0x08810000, READ_U16(0x08810000) & ~(1 << 9)); // Disable IRQ also 
}

#define DOG_TIMEOUT 0x6e

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
    // WRITE_U16(0x088a0000, (READ_U16(0x088a0000) & 0xff00) | DOG_TIMEOUT);
}