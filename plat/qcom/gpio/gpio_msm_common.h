#pragma once
#include "dcc/plat.h"

// GPIO Bitmask
const bitmask GPIO_CFG_ALT_FUNC = {2, 0xf};
const bitmask GPIO_CFG_PULL_TYPE = {0, 0x3};

// GPIO data struct
typedef struct {
    uint32_t no_pins;
    uint32_t pin_start_offset;
    uint32_t gpio_out;
    uint32_t gpio_in;
    uint32_t gpio_oe;
    uint32_t gpio_page;
    uint32_t gpio_cfg;
} GPIOData;