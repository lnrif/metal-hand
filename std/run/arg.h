#ifndef STD_RUN_ARGS_H
#define STD_RUN_ARGS_H

#include "std/run/run.h"
#include "std/str/str.h"

#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

typedef struct {
	u8 const * const * ptr;
	u32 pos;
	u32 len;
	StrZ peek;
} Args;

#elif defined(_WIN32)
// |================================================================================================|
// |> WINDOWS                                                                                       |
// |================================================================================================|

typedef struct {
	u8 const * beg;
	u8 const * at;
} Args;

#endif

Args arg_init(Run const * run);

StrZ arg_peek(Args * arg);
StrZ arg_next(Args * arg);

b8 arg_eof(Args * arg);

#endif // !STD_RUN_ARGS_H
