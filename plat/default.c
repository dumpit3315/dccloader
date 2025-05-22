#include "dcc/plat.h"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
}

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
}