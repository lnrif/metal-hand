#ifndef STD_MEM_CORE_H
#define STD_MEM_CORE_H

#include "std/core.h"

// |================================================================================================|
// |> LIBC BUILTINS                                                                                 |

#define mem_slice_init(ptr, count, value) \
	do { \
		if ((ptr) == 0) break; \
		for (u64 i = 0; i < (count); i += 1) (ptr)[i] = (value); \
	} while (0)

void * memset (void * dst, u8 byte, u64 len);
void * memcpy (void * dst, void const * src, u64 len);
void * memmove(void * dst, void const * src, u64 len);
i32    memcmp (void const * a, void const * b, u64 len);
void * memchr (void const * src, u64 len, u8 byte);

#define memmov memmove

// |================================================================================================|
// |> ALIGN UP/DOWN                                                                                 |

u64 mem_align_up  (u64 value, u64 align);
u64 mem_align_down(u64 value, u64 align);

#endif // !STD_MEM_CORE_H
