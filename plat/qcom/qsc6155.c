#include "dcc/plat.h"

// 80008000 = GPIO_OUT
// 80008010 = GPIO_OE
// 80008020 = GPIO_PAGE
// 80008024 = GPIO_CFG
// 80008028 = GPIO_IN

#define QSC61X5 1
#include "gpio/gpio_qsc6xxx.c"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x80018010, 1);
}

void wdog_reset(void) {
    WRITE_U32(0x8001800c, 1);
    // Reset watchdog (or else, system restarts)
}