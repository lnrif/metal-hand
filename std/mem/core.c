#include "std/mem/core.h"

NO_BUILTIN("memset")
void * memset(void * restrict dst, u8 byte, u64 len) {
	u8 * d = dst;
	#pragma clang loop distribute(disable)
	for (u64 i = 0; i < len; i += 1) d[i] = byte;
	return dst;
};

NO_BUILTIN("memcpy")
void * memcpy(void * restrict dst, void const * restrict src, u64 len) {
	u8       * restrict d = dst;
	u8 const * restrict s = src;
	#pragma clang loop distribute(disable)
	for (u64 i = 0; i < len; i += 1) d[i] = s[i];
	return dst;
};

NO_BUILTIN("memchr")
void * memchr(void const * src, u64 len, u8 byte) {
	u8 const * s = (u8 const *)src;
	#pragma clang loop distribute(disable)
	for (u64 i = 0; i < len; i += 1) if (s[i] == byte) return (void*)(s + i);
	return 0;
};

NO_BUILTIN("memmove")
void * memmove(void * dst, void const * src, u64 n) {
	u8       * d = (u8       *)dst;
	u8 const * s = (u8 const *)src;
	if (d < s) {
		#pragma clang loop distribute(disable)
		for (u64 i = 0; i < n; i += 1) d[i] = s[i];
	} else if (d > s) {
		#pragma clang loop distribute(disable)
		for (u64 i = n; i > 0; i -= 1) d[i - 1] = s[i - 1];
	};
	return dst;
};

NO_BUILTIN("memcmp")
i32 memcmp(void const * a, void const * b, u64 n) {
	u8 const * x = (u8 const *)a;
	u8 const * y = (u8 const *)b;
	#pragma clang loop distribute(disable)
	for (u64 i = 0; i < n; i += 1) {
		if (x[i] == y[i]) continue;
		return x[i] < y[i] ? -1 : 1;
	};
	return 0;
};

