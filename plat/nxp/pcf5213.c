#include "dcc/plat.h"

uint8_t plat_gpio_read(uint32_t pin) {
    // GPIO pin read
    return 0;
}

void plat_gpio_write(uint32_t pin, uint8_t active) {
    // GPIO pin write
}

void plat_gpio_set_dir(uint32_t pin, GPIODirection dir) {
    // GPIO set output direction (oe: 0 = input, 1 = output)
}

void plat_gpio_set_alt_func(uint32_t pin, uint32_t alt_func) {
    // GPIO set alt function
}

void plat_gpio_set_pull(uint32_t pin, GPIOPullType pull) {
    // GPIO set pull (0 = Pull disabled, 1 = Pull up, 2 = Pull down)
}

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    WRITE_U32(0x30503000, 0x0);  // Shut up watchdog
}

#define DOG_TIMEOUT 0x3fff

void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
    // WRITE_U16(0x30503004, 0x8000 | DOG_TIMEOUT);
}