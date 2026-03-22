#include "dcc/plat.h"

#ifndef GPIO_IN
#define GPIO_IN 0x3000660
#endif

#ifndef GPIO_TSEN
#define GPIO_TSEN 0x3000674
#endif

#ifndef GPIO_FUNC
#define GPIO_FUNC 0x3000690
#endif

uint8_t plat_gpio_read(uint32_t pin) {
    // GPIO pin read
    uint8_t gpio_page_offset = (pin >> 4) << 2;
    uint8_t gpio_pin_bit_pos = pin & 0xf;

    return (READ_U16(GPIO_IN + gpio_page_offset) >> gpio_pin_bit_pos) & 1;
}

static uint16_t gpio_buffers[5];
void plat_gpio_write(uint32_t pin, uint8_t active) {
    // GPIO pin write
    uint8_t gpio_page_offset = (pin >> 4) << 2;
    uint8_t gpio_pin_bit_pos = pin & 0xf;
    bitmask gpio_write_bm = {gpio_pin_bit_pos, 1};

    BIT_SET_VAR(gpio_buffers[pin >> 4], gpio_write_bm, active);
    WRITE_U16(GPIO_IN + gpio_page_offset, gpio_buffers[pin >> 4]);
}

static uint16_t gpio_dir_buffers[5];
void plat_gpio_set_dir(uint32_t pin, GPIODirection dir) {
    // GPIO set output direction (oe: 0 = input, 1 = output)
    // GPIO pin write
    uint8_t gpio_page_offset = (pin >> 4) << 2;
    uint8_t gpio_pin_bit_pos = pin & 0xf;
    bitmask gpio_write_bm = {gpio_pin_bit_pos, 1};

    BIT_SET_VAR(gpio_dir_buffers[pin >> 4], gpio_write_bm, dir == GPIO_INPUT ? 0 : 1);
    WRITE_U16(GPIO_TSEN + gpio_page_offset, gpio_dir_buffers[pin >> 4]);
}

static uint16_t gpio_func_sel_buffers[5];
void plat_gpio_set_alt_func(uint32_t pin, uint32_t alt_func) {
    // GPIO set alt function
    uint8_t func_sel_offset = 0;
    uint8_t func_sel_bit_mask = 1;
    uint8_t func_sel_bit_pos = 0;

#if IS_MSM3100 || IS_MSM6000
    switch (pin) {
        /* GPIO select 1*/
        case 37:
            func_sel_bit_pos = 4;
            break;

        case 38:
            func_sel_bit_pos = 3;
            break;

        case 39:
            func_sel_bit_pos = 2;
            break;

        case 40:
            func_sel_bit_pos = 1;
            break;

        case 41:
            func_sel_bit_pos = 0;
            break;

    #if IS_MSM6000
        case 42:
            func_sel_bit_pos = 5;
            func_sel_bit_mask = 0x3;
            break;
    #endif

    #if IS_MSM3100
        case 45:
            func_sel_bit_pos = 8;
            break;

        case 46:
            func_sel_bit_pos = 9;
            break;

        case 47:
            func_sel_bit_pos = 10;
            break;
    #endif

        /* GPIO select 2 */

        case 12:
            func_sel_bit_pos = 13;
            func_sel_offset = 1;
            break;

    #if IS_MSM3100
        case 13:
            func_sel_bit_pos = 2;
            func_sel_offset = 1;
            break;
    #endif

        case 18:
        case 19:
        case 20:
        case 21:
            func_sel_bit_pos = 0;
            func_sel_offset = 1;
            break;

        case 30:
            func_sel_bit_pos = 3;
            func_sel_offset = 1;
            func_sel_bit_mask = 0x7;
            break;

        default:
            return;
    }
#else
    switch (pin) {
        case 26:
            func_sel_bit_pos = 5;
            break;

        case 29:
            func_sel_bit_pos = 0;
            break;

        case 30:
            func_sel_bit_pos = 1;
            func_sel_bit_mask = 0x7;
            break;

        default:
            return;
    }
#endif

    uint8_t gpio_page_offset = func_sel_offset << 2;
    bitmask gpio_write_bm = {func_sel_bit_pos, func_sel_bit_mask};

    BIT_SET_VAR(gpio_func_sel_buffers[func_sel_offset], gpio_write_bm, alt_func);
    WRITE_U16(GPIO_FUNC + gpio_page_offset, gpio_func_sel_buffers[func_sel_offset]);
}

void plat_gpio_set_pull(uint32_t pin, GPIOPullType pull) {
    // GPIO set pull (0 = Pull disabled, 1 = Pull up, 2 = Pull down)
}

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x03000600, 0x10);
    // WRITE_U32(0x03000600, 0);
}

void wdog_reset(void) {
    WRITE_U32(0x03000600, 1);
    WRITE_U32(0x03000600, 0);
    // Reset watchdog (or else, system restarts)
}