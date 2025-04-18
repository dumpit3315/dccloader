#pragma once
#include <stdint.h>

typedef struct {
    uint32_t bit_pos;
    uint32_t bit_mask;    
} bitmask;

uint8_t GET_BIT8(uint32_t offset, bitmask bitmask);
uint16_t GET_BIT16(uint32_t offset, bitmask bitmask);
uint32_t GET_BIT32(uint32_t offset, bitmask bitmask);

void SET_BIT8(uint32_t offset, bitmask bitmask, uint8_t value);
void SET_BIT16(uint32_t offset, bitmask bitmask, uint16_t value);
void SET_BIT32(uint32_t offset, bitmask bitmask, uint32_t value);
