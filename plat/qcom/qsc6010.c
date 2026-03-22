#include "dcc/plat.h"

#define GPIO1_OFFSET 0x88006000
#define GPIO2_OFFSET 0x88008000
#define QSC60x0 1
#include "gpio/gpio_msm62xx.c"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x88004010, 1);
}

void wdog_reset(void) {
    WRITE_U32(0x8800400c, 1);
    // Reset watchdog (or else, system restarts)
}