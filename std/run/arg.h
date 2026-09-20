#ifndef STD_RUN_ARGS_H
#define STD_RUN_ARGS_H

#include "std/run/run.h"
#include "std/str/core.h"

#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

typedef struct {
	u8 const * const * ptr;
	u32 pos;
	u32 len;
	Str peek;
} Args;

#define ARG_PEEK_EMBED(args, _pad, peek) \
	union { \
		Args args; \
		struct { u8 _pad[8 + 4 + 4]; Str peek; };\
	}

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

// Str arg_peek(Args * args);
Str arg_next(Args * args);
#define arg_skip(args) UNUSED(arg_next(args))
#define arg_peek(args) ((args)->peek)

b8 arg_eof(Args * args);

#endif // !STD_RUN_ARGS_H
