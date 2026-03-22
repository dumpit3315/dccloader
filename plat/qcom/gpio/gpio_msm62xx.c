#include "gpio_msm_common.h"

#ifndef GPIO1_OFFSET
#define GPIO1_OFFSET 0x80000900
#endif

#ifndef GPIO2_OFFSET
#define GPIO2_OFFSET 0x80004400
#endif

// GPIO1 Registers
#if MSM6100 || MSM6500
#define REG_GPIO_OUT_0 0x0000
#define REG_GPIO_OUT_1 0x0004
#define REG_GPIO_OUT_4 0x0008
#define REG_GPIO_OE_0  0x000c
#define REG_GPIO_OE_1  0x0010
#define REG_GPIO_OE_4  0x0014
#define REG_GPIO_PAGE  0x0020
#define REG_GPIO_CFG   0x0024
#define REG_GPIO_IN_0  0x0034
#define REG_GPIO_IN_1  0x0038
#define REG_GPIO_IN_4  0x003c
#elif QSC60x0
#define REG_GPIO_OUT_0 0x0000
#define REG_GPIO_IN_0  0x0004
#define REG_GPIO_OE_0  0x0008
#define REG_GPIO_PAGE  0x000C
#define REG_GPIO_CFG   0x0010
#else
#define REG_GPIO_OUT_0 0x0000
#define REG_GPIO_OUT_1 0x0004
#define REG_GPIO_OUT_4 0x0008
#define REG_GPIO_OUT_5 0x000c
#define REG_GPIO_OE_0  0x0010
#define REG_GPIO_OE_1  0x0014
#define REG_GPIO_OE_4  0x0018
#define REG_GPIO_OE_5  0x001c
#define REG_GPIO_PAGE  0x0020
#define REG_GPIO_CFG   0x0024
#define REG_GPIO_IN_0  0x0034
#define REG_GPIO_IN_1  0x0038
#define REG_GPIO_IN_4  0x003c
#define REG_GPIO_IN_5  0x0040
#endif

// GPIO2 Registers
#if MSM6100
#define REG_GPIO2_OE_2  0x0000
#define REG_GPIO2_OE_3  0x0004
#define REG_GPIO2_OUT_2 0x0008
#define REG_GPIO2_OUT_3 0x000c
#define REG_GPIO2_PAGE  0x0040
#define REG_GPIO2_CFG   0x0044
#define REG_GPIO2_IN_2  0x0060
#define REG_GPIO2_IN_3  0x0064
#elif QSC60x0
#define REG_GPIO2_OUT_2 0x0000
#define REG_GPIO2_OUT_3 0x0004
#define REG_GPIO2_IN_2  0x0008
#define REG_GPIO2_IN_3  0x000c
#define REG_GPIO2_OE_2  0x0010
#define REG_GPIO2_OE_3  0x0014
#define REG_GPIO2_PAGE  0x0018
#define REG_GPIO2_CFG   0x001c
#else
#define REG_GPIO2_OE_2  0x0000
#define REG_GPIO2_OE_3  0x0004
#define REG_GPIO2_OUT_2 0x0008
#define REG_GPIO2_OUT_3 0x000c
#define REG_GPIO2_PAGE  0x0030
#define REG_GPIO2_CFG   0x0034
#define REG_GPIO2_IN_2  0x0040
#define REG_GPIO2_IN_3  0x0044
#endif

// GPIOs
#if (MSM6100 || MSM6500)
const GPIOData gpios[5] = {
    {
        .no_pins = 32,
        .pin_start_offset = 0,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_0,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_0,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_0,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 7,
        .pin_start_offset = 32,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_1,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_1,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_1,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 16,
        .pin_start_offset = 39,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_2,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_2,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_2,
        .gpio_page = GPIO2_OFFSET + REG_GPIO2_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO2_CFG
    },
    {
        .no_pins = 12,
        .pin_start_offset = 55,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_3,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_3,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_3,
        .gpio_page = GPIO2_OFFSET + REG_GPIO2_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO2_CFG
    },
    {
        .no_pins = 32,
        .pin_start_offset = 67,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_4,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_4,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_4,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
};
#elif QSC60x0
const GPIOData gpios[3] = {
    {
        .no_pins = 21,
        .pin_start_offset = 0,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_0,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_0,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_0,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 32,
        .pin_start_offset = 21,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_2,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_2,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_2,
        .gpio_page = GPIO2_OFFSET + REG_GPIO2_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO2_CFG
    },
    {
        .no_pins = 4,
        .pin_start_offset = 53,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_3,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_3,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_3,
        .gpio_page = GPIO2_OFFSET + REG_GPIO2_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO2_CFG
    }
};
#else
const GPIOData gpios[6] = {
    {
        .no_pins = 32,
        .pin_start_offset = 0,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_0,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_0,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_0,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 7,
        .pin_start_offset = 32,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_1,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_1,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_1,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 16,
        .pin_start_offset = 39,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_2,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_2,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_2,
        .gpio_page = GPIO2_OFFSET + REG_GPIO2_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO2_CFG
    },
    {
        .no_pins = 12,
        .pin_start_offset = 55,
        .gpio_out = GPIO2_OFFSET + REG_GPIO2_OUT_3,
        .gpio_in = GPIO2_OFFSET + REG_GPIO2_IN_3,
        .gpio_oe = GPIO2_OFFSET + REG_GPIO2_OE_3,
        .gpio_page = GPIO2_OFFSET + REG_GPIO2_PAGE,
        .gpio_cfg = GPIO2_OFFSET + REG_GPIO2_CFG
    },
    {
        .no_pins = 32,
        .pin_start_offset = 67,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_4,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_4,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_4,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
    {
        .no_pins = 16,
        .pin_start_offset = 99,
        .gpio_out = GPIO1_OFFSET + REG_GPIO_OUT_5,
        .gpio_in = GPIO1_OFFSET + REG_GPIO_IN_5,
        .gpio_oe = GPIO1_OFFSET + REG_GPIO_OE_5,
        .gpio_page = GPIO1_OFFSET + REG_GPIO_PAGE,
        .gpio_cfg = GPIO1_OFFSET + REG_GPIO_CFG
    },
};
#endif

#define GPIO_TOTAL_LEN (sizeof(gpios) / sizeof(GPIOData))
#include "gpio_msm_common.c"