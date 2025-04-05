#pragma once
#include <stdint.h>

#define WRITE_U8(_reg, _val)  (*((volatile uint8_t *)(_reg)) = (_val))
#define WRITE_U16(_reg, _val)  (*((volatile uint16_t *)(_reg)) = (_val))
#define WRITE_U32(_reg, _val)  (*((volatile uint32_t *)(_reg)) = (_val))

#define READ_U8(_reg)   (*((volatile uint8_t *)(_reg)))
#define READ_U16(_reg)   (*((volatile uint16_t *)(_reg)))
#define READ_U32(_reg)   (*((volatile uint32_t *)(_reg)))