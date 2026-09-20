#include "std/run/env.h"
#include "std/str/core.h"

#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

Env env_init(Run const * restrict run) {
	Env env = (Env){.ptr = run->env, .pos = 0, .peek = ENV_ARG_NIL};
	env_next(&env);
	return env;
};

EnvArg env_peek(Env * restrict env) {
	return env->peek;
};

static EnvArg env_parse(u8z const * str) {
	StrZ const full = str_z_init(str);
	Str        name = STR_NIL;
	StrZ       expr = STR_Z_NIL;

	for (u64 eq = 0; eq < full.len; eq += 1) {
		if (full.raw[eq] != '=') continue;
		name = (Str ){.raw = &full.raw[0],      .len = eq};
		expr = (StrZ){.raw = &full.raw[eq + 1], .len = full.len - eq - 1};
		break;
	};

	return (EnvArg){.full = full, .name = name, .expr = expr};
};

EnvArg env_next(Env * restrict env) {
	EnvArg const peek = env->peek;

	if (env->ptr[env->pos] != 0) {
		env->peek = env_parse(env->ptr[env->pos]);
		env->pos += 1;
	} else {
		env->peek = ENV_ARG_NIL;
	};

	return peek;
};

b8 env_eof(Env const * restrict env) {
	return str_nil(env->peek.full);
};

#elif defined(_WIN32)
// |================================================================================================|
// |> WINDOWS                                                                                       |
// |================================================================================================|

// Args args_init(Run const * restrict run) {};
// b8   args_eof(Args const * restrict args) {};
// StrZ args_next(Args * restrict args) {};

#else

#error "unsupported OS"

#endif

