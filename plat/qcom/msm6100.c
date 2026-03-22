#include "dcc/plat.h"

#define GPIO1_OFFSET 0x80001200
#define GPIO2_OFFSET 0x58000000
#define MSM6100 1
#define GPIO_NO_KEEPER 1
#include "gpio/gpio_msm62xx.c"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x80000700, 0x10);
    // WRITE_U32(0x80000700, 0);
}

void wdog_reset(void) {
    WRITE_U32(0x80000700, 1);
    WRITE_U32(0x80000700, 0);
    // Reset watchdog (or else, system restarts)
}