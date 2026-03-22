#include "dcc/plat.h"

const uint32_t gpio_read_table[4] = {0x03000720, 0x03000724, 0x09000060, 0x09000064};
const uint32_t gpio_write_table[4] = {0x030006e8, 0x030006ec, 0x09000008, 0x0900000c};
const uint32_t gpio_oe_table[4] = {0x030006f4, 0x030006f8, 0x09000000, 0x09000004};
const uint32_t gpio_func_sel_table[4] = {0x03000700, 0x03000704, 0x0, 0x0};

void plat_gpio_get_reg_offset(uint32_t pin, uint8_t *reg_index, uint8_t *reg_bit_offset) {
  if (pin < 32) {
    *reg_index = 0;
  } else if (pin < 43) {
    *reg_index = 1;
    pin -= 32;
  } else if (pin < 59) {
    *reg_index = 2;
    pin -= 43;
  } else {
    *reg_index = 3;
    pin -= 59;
  }

  *reg_bit_offset = pin;
}

#define FUNC_SEL_CONDITION (pin < 43)
#include "gpio/gpio_msm6x50.c"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x030006d0, 0x10);
    // WRITE_U32(0x030006d0, 0);
}

void wdog_reset(void) {
    WRITE_U32(0x030006d0, 1);
    WRITE_U32(0x030006d0, 0);
    // Reset watchdog (or else, system restarts)
}