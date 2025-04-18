#include "dcc/plat.h"
#include "bitutils.h"

/*
 * Bit Helpers
*/

/* 32 */
static uint32_t _get_bit32(uint32_t offset, uint32_t bit_position, uint32_t bit_mask)
{
    return (READ_U32(offset) >> bit_position) & bit_mask;
}

static void _set_bit32(uint32_t offset, uint32_t bit_position, uint32_t bit_mask, uint32_t value)
{
    WRITE_U32(offset, (READ_U32(offset) & ~(bit_mask << bit_position)) | ((value & bit_mask) << bit_position));
}

uint32_t GET_BIT32(uint32_t offset, bitmask bitmask)
{
    return _get_bit32(offset, bitmask.bit_pos, bitmask.bit_mask);
}

void SET_BIT32(uint32_t offset, bitmask bitmask, uint32_t value)
{
    _set_bit32(offset, bitmask.bit_pos, bitmask.bit_mask, value);
}

/* 16 */
static uint16_t _get_bit16(uint32_t offset, uint32_t bit_position, uint32_t bit_mask)
{
    return (READ_U16(offset) >> bit_position) & bit_mask;
}

static void _set_bit16(uint32_t offset, uint32_t bit_position, uint32_t bit_mask, uint16_t value)
{
    WRITE_U16(offset, (READ_U16(offset) & ~(bit_mask << bit_position)) | ((value & bit_mask) << bit_position));
}


uint16_t GET_BIT16(uint32_t offset, bitmask bitmask)
{
    return _get_bit16(offset, bitmask.bit_pos, bitmask.bit_mask);
}

void SET_BIT16(uint32_t offset, bitmask bitmask, uint16_t value)
{
    _set_bit16(offset, bitmask.bit_pos, bitmask.bit_mask, value);
}


/* 8 */
static uint8_t _get_bit8(uint32_t offset, uint32_t bit_position, uint32_t bit_mask)
{
    return (READ_U8(offset) >> bit_position) & bit_mask;
}

static void _set_bit8(uint32_t offset, uint32_t bit_position, uint32_t bit_mask, uint8_t value)
{
    WRITE_U8(offset, (READ_U8(offset) & ~(bit_mask << bit_position)) | ((value & bit_mask) << bit_position));
}

uint8_t GET_BIT8(uint32_t offset, bitmask bitmask)
{
    return _get_bit8(offset, bitmask.bit_pos, bitmask.bit_mask);
}

void SET_BIT8(uint32_t offset, bitmask bitmask, uint8_t value)
{
    _set_bit8(offset, bitmask.bit_pos, bitmask.bit_mask, value);
}