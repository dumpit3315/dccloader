#include "dcc/plat.h"

const uint32_t gpio_read_table[5] = {0x84000168, 0x8400016c, 0x48000060, 0x48000064, 0x84000170};
const uint32_t gpio_write_table[5] = {0x8400015c, 0x84000160, 0x48000008, 0x4800000c, 0x84000164};
const uint32_t gpio_oe_table[5] = {0x84000150, 0x84000154, 0x48000000, 0x48000004, 0x84000158};
const uint32_t gpio_func_sel_table[5] = {0x84000174, 0x84000178, 0x0, 0x0, 0x8400017c};

void plat_gpio_get_reg_offset(uint32_t pin, uint8_t *reg_index, uint8_t *reg_bit_offset) {
  if (pin < 32) {
    *reg_index = 0;
  } else if (pin < 39) {
    *reg_index = 1;
    pin -= 32;
  } else if (pin < 55) {
    *reg_index = 2;
    pin -= 39;
  } else if (pin < 67) {
    *reg_index = 3;
    pin -= 55;
  } else {
    *reg_index = 4;
    pin -= 67;
  }
  
  *reg_bit_offset = pin;
}

#define FUNC_SEL_CONDITION (pin < 39 || pin >= 67)
#include "gpio/gpio_msm6x50.c"

void plat_init(void) {
    // Initialize platform (after CMM, H/W init script, TCL, etc, and Uploading)
    // Can alternatively be turned off
    // WRITE_U32(0x80003404, 0x10);
    // WRITE_U32(0x80003404, 0);
}

void wdog_reset(void) {
    WRITE_U32(0x80003404, 1);
    WRITE_U32(0x80003404, 0);
    // Reset watchdog (or else, system restarts)
}