#include "std/run/arg.h"

#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

Args arg_init(Run const * restrict run) {
	Args arg = (Args){
		.ptr = run->args, .pos = 0,
		.len = run->args_len,
		.peek = STR_Z_NIL,
	}; arg_next(&arg);
	return arg;
};

StrZ arg_peek(Args * restrict arg) {
	return arg->peek;
};

StrZ arg_next(Args * restrict arg) {
	StrZ const peek = arg->peek;

	if (arg->pos < arg->len) {
		arg->peek = str_z_init(arg->ptr[arg->pos]);
		arg->pos += 1;
	} else {
		arg->peek = STR_Z_NIL;
	};

	return peek;
};

b8 arg_eof(Args * restrict arg) {
	return str_is_nil(&arg->peek);
};

#elif defined(_WIN32)
// |================================================================================================|
// |> WINDOWS                                                                                       |
// |================================================================================================|

// Args arg_init(Run const * restrict run) {};
// b8   arg_eof(Args const * restrict arg) {};
// StrZ arg_next(Args * restrict arg) {};

#else

#error "unsupported OS"

#endif

