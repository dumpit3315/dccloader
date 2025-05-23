#pragma once
#include <stdint.h>
#include <stddef.h>
#include <memory.h>

#ifdef DCC_TESTING
#include <stdio.h>

extern void WRITE_U8(uint32_t reg, uint8_t val);
extern void WRITE_U16(uint32_t reg, uint16_t val);
extern void WRITE_U32(uint32_t reg, uint32_t val);

extern uint8_t READ_U8(uint32_t reg);
extern uint16_t READ_U16(uint32_t reg);
extern uint32_t READ_U32(uint32_t reg);

extern void *PLAT_MEMCPY(void *dest, const void *src, size_t n);
#define INT_MEMCPY memcpy
#define PLAT_SNPRINTF snprintf
#else
#include "dcc/lwprintf.h"

#define WRITE_U8(_reg, _val)  (*((volatile uint8_t *)(_reg)) = (_val))
#define WRITE_U16(_reg, _val)  (*((volatile uint16_t *)(_reg)) = (_val))
#define WRITE_U32(_reg, _val)  (*((volatile uint32_t *)(_reg)) = (_val))

#define READ_U8(_reg)   (*((volatile uint8_t *)(_reg)))
#define READ_U16(_reg)   (*((volatile uint16_t *)(_reg)))
#define READ_U32(_reg)   (*((volatile uint32_t *)(_reg)))

#define PLAT_MEMCPY memcpy
#define INT_MEMCPY memcpy
#define PLAT_SNPRINTF lwprintf_snprintf
#endif