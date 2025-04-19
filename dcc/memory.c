#include <stddef.h>
#include "dn_dcc_proto.h"

void *memcpy(void *dst, const void *src, size_t len)
{
	const char *s = src;
	char *d = dst;

	while (len--) {
        if ((len & 0x1ff) == 0x1ff) wdog_reset();
		*d++ = *s++;
    }

	return dst;
}

void *memset(void *dst, int val, size_t count)
{
	register uint8_t *ptr = (uint8_t *)dst;

	while (count-- > 0) {
		if ((count & 0x1ff) == 0x1ff) wdog_reset();
	  	*ptr++ = val;
	}

	return dst;
}

void *memmove(void *dst, const void *src, size_t len)
{
	/*
	 * The following test makes use of unsigned arithmetic overflow to
	 * more efficiently test the condition !(src <= dst && dst < str+len).
	 * It also avoids the situation where the more explicit test would give
	 * incorrect results were the calculation str+len to overflow (though
	 * that issue is probably moot as such usage is probably undefined
	 * behaviour and a bug anyway.
	 */
	if ((size_t)dst - (size_t)src >= len) {
		/* destination not in source data, so can safely use memcpy */
		return memcpy(dst, src, len);
	} else {
		/* copy backwards... */
		const char *end = dst;
		const char *s = (const char *)src + len;
		char *d = (char *)dst + len;
		while (d != end) {
            if (((size_t)d & 0x1ff) == 0x1ff) wdog_reset();
			*--d = *--s;
        }
	}
	return dst;
}

int memcmp(const void *s1, const void *s2, size_t len)
{
	const unsigned char *s = s1;
	const unsigned char *d = s2;
	unsigned char sc;
	unsigned char dc;

	while (len--) {
        if ((len & 0x1ff) == 0x1ff) wdog_reset();
		sc = *s++;
		dc = *d++;
		if (sc - dc)
			return (sc - dc);
	}

	return 0;
}