#ifndef STD_RUN_ENV_H
#define STD_RUN_ENV_H

#include "std/run/run.h"
#include "std/str/core.h"

typedef struct {
	StrZ full;
	Str  name;
	StrZ expr;
} EnvArg;

#define ENV_ARG_NIL     ((EnvArg){.full = STR_Z_NIL, .name = STR_NIL, .expr = STR_Z_NIL})
#define ENV_IS_NIL(arg) str_is_nil((arg)->full)

#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

typedef struct {
	u8 const * const * ptr;
	u32 pos;
	EnvArg peek;
} Env;

#elif defined(_WIN32)
// |================================================================================================|
// |> WINDOWS                                                                                       |
// |================================================================================================|

typedef struct {
	
} Env;

#endif

// TODO: add allocator
Env env_init(Run const * run);

b8     env_eof(Env const * env);
EnvArg env_peek(Env * env);
EnvArg env_next(Env * env);

#endif // !STD_RUN_ENV_H
