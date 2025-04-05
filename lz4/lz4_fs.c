#include <memory.h>

#define LZ4_FREESTANDING 1
#define LZ4_memcpy memcpy
#define LZ4_memset memset
#define LZ4_memmove memmove

#include "lz4.c"