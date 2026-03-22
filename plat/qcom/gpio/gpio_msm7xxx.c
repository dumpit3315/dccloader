#include "dcc/plat.h"
#include "gpio_msm_common.h"

#ifndef GPIO1_OFFSET
#define GPIO1_OFFSET 0xA9000000
#endif

#ifndef GPIO2_OFFSET
#define GPIO2_OFFSET 0xA9100400
#endif

// GPIO1 Registers
#define REG_GPIO_OUT_0 0x0000
#define REG_GPIO_OUT_2 0x0004
#define REG_GPIO_OUT_3 0x0008
#define REG_GPIO_OUT_4 0x000c
#define REG_GPIO_OE_0  0x0010
#define REG_GPIO_OE_2  0x0014
#define REG_GPIO_OE_3  0x0018
#define REG_GPIO_OE_4  0x001c
#define REG_GPIO_PAGE  0x0020
#define REG_GPIO_CFG   0x0024
#define REG_GPIO_IN_0  0x0034
#define REG_GPIO_IN_2  0x0038
#define REG_GPIO_IN_3  0x003c
#define REG_GPIO_IN_4  0x0040

// GPIO2 Registers
#define REG_GPIO2_OUT_1 0x0000
#define REG_GPIO2_OE_1 0x0008
#define REG_GPIO2_PAGE  0x0010
#define REG_GPIO2_CFG   0x0014
#define REG_GPIO2_IN_1  0x0020

// GPIOs
const GPIOData gpios[5] = {
    {
        .no_pins = 16,
        .pin_start_offset = 0,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_0,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_0,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_0,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 27,
        .pin_start_offset = 16,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_1,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_1,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_1,
        .gpio_page = GPIO2_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 25,
        .pin_start_offset = 43,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_2,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_2,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_2,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 27,
        .pin_start_offset = 68,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_3,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_3,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_3,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 24,
        .pin_start_offset = 95,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_4,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_4,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_4,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    }
};

#define GPIO_TOTAL_LEN (sizeof(gpios) / sizeof(GPIOData))
#include "gpio_msm_common.c"