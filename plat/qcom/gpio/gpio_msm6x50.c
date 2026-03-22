#include "dcc/plat.h"

extern const uint32_t gpio_read_table[];
extern const uint32_t gpio_write_table[];
extern const uint32_t gpio_oe_table[];
extern const uint32_t gpio_func_sel_table[];

void plat_gpio_get_reg_offset(uint32_t pin, uint8_t *reg_index, uint8_t *reg_bit_offset);

uint8_t plat_gpio_read(uint32_t pin) {
    // GPIO pin read
    uint8_t reg_index;
    uint8_t pin_bit_offset;
    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);

    return (READ_U32(gpio_read_table[reg_index]) >> pin_bit_offset) & 1;
}

static uint32_t gpio_buffers[8];
void plat_gpio_write(uint32_t pin, uint8_t active) {
    // GPIO pin write
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);
    bitmask gpio_write_bm = {pin_bit_offset, 1};

    BIT_SET_VAR(gpio_buffers[reg_index], gpio_write_bm, active);
    WRITE_U32(gpio_write_table[reg_index], gpio_buffers[reg_index]);
}

static uint32_t gpio_dir_buffers[8];
void plat_gpio_set_dir(uint32_t pin, GPIODirection dir) {
    // GPIO set output direction (oe: 0 = input, 1 = output)
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);
    bitmask gpio_write_bm = {pin_bit_offset, 1};

    BIT_SET_VAR(gpio_dir_buffers[reg_index], gpio_write_bm, dir == GPIO_INPUT ? 0 : 1);
    WRITE_U32(gpio_oe_table[reg_index], gpio_dir_buffers[reg_index]);
}

static uint32_t gpio_func_sel_buffers[8];
void plat_gpio_set_alt_func(uint32_t pin, uint32_t alt_func) {
    // GPIO set alt function
    uint8_t reg_index;
    uint8_t pin_bit_offset;

    if (FUNC_SEL_CONDITION) {
        plat_gpio_get_reg_offset(pin, &reg_index, &pin_bit_offset);
        bitmask gpio_write_bm = {pin_bit_offset, 1};

        BIT_SET_VAR(gpio_func_sel_buffers[reg_index], gpio_write_bm, alt_func);
        WRITE_U32(gpio_func_sel_table[reg_index], gpio_func_sel_buffers[reg_index]);
    }
}

void plat_gpio_set_pull(uint32_t pin, GPIOPullType pull) {
    // GPIO set pull (0 = Pull disabled, 1 = Pull up, 2 = Pull down)
}