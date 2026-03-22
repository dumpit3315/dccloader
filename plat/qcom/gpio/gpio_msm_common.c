#include "gpio_msm_common.h"

#ifndef GPIO_TOTAL_LEN
#define GPIO_TOTAL_LEN 6
#endif

extern const GPIOData gpios[];

void plat_gpio_get_reg_offset(uint32_t pin, uint8_t *reg_index, uint8_t *reg_bits) {
    for (int i = 0; i < GPIO_TOTAL_LEN; i++) {
        if (pin >= gpios[i].pin_start_offset && pin < (gpios[i].pin_start_offset + gpios[i].no_pins)) {
            *reg_index = i;
            *reg_bits = pin - gpios[i].pin_start_offset;
            return;
        }
    }

    *reg_index = 0;
    *reg_bits = 0;
}

uint8_t plat_gpio_read(uint32_t pin) {
    // GPIO pin read
    uint8_t reg_index;
    uint8_t pin_bit_offset;
    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);

    return (READ_U32(gpios[reg_index].gpio_in) >> pin_bit_offset) & 1;
}

static uint32_t gpio_buffers[8];
void plat_gpio_write(uint32_t pin, uint8_t active) {
    // GPIO pin write
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);
    bitmask gpio_write_bm = {pin_bit_offset, 1};

    BIT_SET_VAR(gpio_buffers[reg_index], gpio_write_bm, active);
    WRITE_U32(gpios[reg_index].gpio_out, gpio_buffers[reg_index]);
}

static uint32_t gpio_dir_buffers[8];
void plat_gpio_set_dir(uint32_t pin, GPIODirection dir) {
    // GPIO set output direction (oe: 0 = input, 1 = output)
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);
    bitmask gpio_write_bm = {pin_bit_offset, 1};

    BIT_SET_VAR(gpio_dir_buffers[reg_index], gpio_write_bm, dir == GPIO_INPUT ? 0 : 1);
    WRITE_U32(gpios[reg_index].gpio_oe, gpio_dir_buffers[reg_index]);
}

static uint32_t gpio_cfg_buffers[128];
void plat_gpio_set_alt_func(uint32_t pin, uint32_t alt_func) {
    // GPIO set alt function
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);

    BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_ALT_FUNC, alt_func);

    WRITE_U32(gpios[reg_index].gpio_page, pin);
    WRITE_U32(gpios[reg_index].gpio_cfg, gpio_cfg_buffers[pin]);
}

// The keeper keeps the output level the same even if you turn the driver off.
void plat_gpio_set_pull(uint32_t pin, GPIOPullType pull) {
    // GPIO set pull (0 = Pull disabled, 1 = Pull up, 2 = Pull down)
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);

    switch (pull) {
        case GPIO_PULL_DISABLED:
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b00);
            break;

        case GPIO_PULL_UP:
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b11);
            break;

        case GPIO_PULL_DOWN:
        #if GPIO_NO_KEEPER
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b10);
        #else
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b01);
        #endif
            break;
    }

    WRITE_U32(gpios[reg_index].gpio_page, pin);
    WRITE_U32(gpios[reg_index].gpio_cfg, gpio_cfg_buffers[pin]);
}