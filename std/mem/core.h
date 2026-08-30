#ifndef STD_MEM_CORE_H
#define STD_MEM_CORE_H

#include "std/core.h"

void * memset(void * dst, u8 byte, u64 len);
void * memcpy(void * dst, void const * src, u64 len);
void * memmove(void * dst, void const * src, u64 len);
i32    memcmp(void const * a, void const * b, u64 len);
void * memchr(void const * src, u64 len, u8 byte);

#endif // !STD_MEM_CORE_H
