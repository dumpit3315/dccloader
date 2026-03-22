#include "dcc/plat.h"
#include "gpio_msm_common.h"

#ifndef GPIO1_OFFSET
#define GPIO1_OFFSET 0x8000A000
#endif

#ifndef GPIO2_OFFSET
#define GPIO2_OFFSET 0x84022000
#endif

// GPIO Registers
#if QSC60X5 | QSC11XX
#define REG_GPIO_OUT 0x0000
#define REG_GPIO_OE  0x0008
#define REG_GPIO_PAGE  0x0010
#define REG_GPIO_CFG   0x0014
#define REG_GPIO_IN  0x0018
#elif QSC61X5
#define REG_GPIO_OUT 0x0000
#define REG_GPIO_OE  0x0010
#define REG_GPIO_PAGE  0x0020
#define REG_GPIO_CFG   0x0024
#define REG_GPIO_IN  0x0028
#else
#define REG_GPIO_OUT 0x0000
#define REG_GPIO_OE  0x000c
#define REG_GPIO_PAGE  0x0018
#define REG_GPIO_CFG   0x001c
#define REG_GPIO_IN  0x0020
#endif

// GPIO masks
// #define GPIO_MASK_01  0x1f801fe
// #define GPIO_MASK_02  0xffffe1c0
// #define GPIO_MASK_03  0x3fff
#if QSC60X5
const uint32_t gpio_mask_index[2] = {0x7c0037fe, 0x3fdc009};
#elif QSC11XX
const uint32_t gpio_mask_index[2] = {0x7fffd000, 0x7fa003fc};
#elif QSC61X5
const uint32_t gpio_mask_index[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
#else
const uint32_t gpio_mask_index[3] = {0x1f801fe, 0xffffe1c0, 0x3fff};
#endif

inline void plat_gpio_get_base(uint32_t pin, uint32_t reg_offset, uint32_t *reg_conv_offset, uint32_t *reg_mask) {
    uint32_t pin_bit_offset = pin & 0x1f;
    uint32_t mask_index = pin >> 5;
    uint32_t base_offset = (gpio_mask_index[mask_index] & (1 << pin_bit_offset)) ? GPIO1_OFFSET : GPIO2_OFFSET; // 1 - GPIO1, 0 - GPIO2

    *reg_conv_offset = base_offset + (mask_index << 2) + reg_offset;
    *reg_mask = (gpio_mask_index[mask_index] & (1 << pin_bit_offset)) ? gpio_mask_index[mask_index] : ~(gpio_mask_index[mask_index]);
}

uint8_t plat_gpio_read(uint32_t pin) {
    // GPIO pin read
    uint32_t gpio_reg_offset;
    uint32_t gpio_reg_mask;
    uint8_t pin_bit_offset = pin & 0x1f;

    plat_gpio_get_base(pin, REG_GPIO_IN, &gpio_reg_offset, &gpio_reg_mask);

    return (READ_U32(gpio_reg_offset) >> pin_bit_offset) & 1;
}

static uint32_t gpio_buffers[4];
void plat_gpio_write(uint32_t pin, uint8_t active) {
    // GPIO pin write
    uint32_t gpio_reg_offset;
    uint32_t gpio_reg_mask;
    uint8_t pin_bit_offset = pin & 0x1f;
    uint32_t mask_index = pin >> 5;

    plat_gpio_get_base(pin, REG_GPIO_OUT, &gpio_reg_offset, &gpio_reg_mask);
    bitmask gpio_write_bm = {pin_bit_offset, 1};

    BIT_SET_VAR(gpio_buffers[mask_index], gpio_write_bm, active);
    WRITE_U32(gpio_reg_offset, gpio_buffers[mask_index] & gpio_reg_mask);
}

static uint32_t gpio_dir_buffers[4];
void plat_gpio_set_dir(uint32_t pin, GPIODirection dir) {
    // GPIO set output direction (oe: 0 = input, 1 = output)
    uint32_t gpio_reg_offset;
    uint32_t gpio_reg_mask;
    uint8_t pin_bit_offset = pin & 0x1f;
    uint32_t mask_index = pin >> 5;

    plat_gpio_get_base(pin, REG_GPIO_OE, &gpio_reg_offset, &gpio_reg_mask);
    bitmask gpio_write_bm = {pin_bit_offset, 1};

    BIT_SET_VAR(gpio_dir_buffers[mask_index], gpio_write_bm, dir == GPIO_INPUT ? 0 : 1);
    WRITE_U32(gpio_reg_offset, gpio_dir_buffers[mask_index] & gpio_reg_mask);
}

static uint32_t gpio_cfg_buffers[128];
void plat_gpio_set_alt_func(uint32_t pin, uint32_t alt_func) {
    // GPIO set alt function
    uint32_t gpio_reg_offset_page;
    uint32_t gpio_reg_offset_cfg;
    uint32_t gpio_reg_mask;

    plat_gpio_get_base(pin, REG_GPIO_PAGE, &gpio_reg_offset_page, &gpio_reg_mask);
    plat_gpio_get_base(pin, REG_GPIO_CFG, &gpio_reg_offset_cfg, &gpio_reg_mask);

    BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_ALT_FUNC, alt_func);

    WRITE_U32(gpio_reg_offset_page, pin);
    WRITE_U32(gpio_reg_offset_cfg, gpio_cfg_buffers[pin]);
}

// The keeper keeps the output level the same even if you turn the driver off.
void plat_gpio_set_pull(uint32_t pin, GPIOPullType pull) {
    // GPIO set pull (0 = Pull disabled, 1 = Pull up, 2 = Pull down)
    uint32_t gpio_reg_offset_page;
    uint32_t gpio_reg_offset_cfg;
    uint32_t gpio_reg_mask;

    plat_gpio_get_base(pin, REG_GPIO_PAGE, &gpio_reg_offset_page, &gpio_reg_mask);
    plat_gpio_get_base(pin, REG_GPIO_CFG, &gpio_reg_offset_cfg, &gpio_reg_mask);

    switch (pull) {
        case GPIO_PULL_DISABLED:
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b00);
            break;

        case GPIO_PULL_UP:
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b11);
            break;

        case GPIO_PULL_DOWN:
            BIT_SET_VAR(gpio_cfg_buffers[pin], GPIO_CFG_PULL_TYPE, 0b01);
            break;
    }

    WRITE_U32(gpio_reg_offset_page, pin);
    WRITE_U32(gpio_reg_offset_cfg, gpio_cfg_buffers[pin]);
}