#include "risk/src/args/args.h"
#include "std/fmt/core.h"

ArgsResult args_common_try(ArgsState * state) {
	if (str_eq_lit(state->peek, "-h") || str_eq_lit(state->peek, "--help")) {
		state->help = true;
		goto known;
	};

	if (str_eq_lit(state->peek, "-c") || str_eq_lit(state->peek, "--color")) {
		state->color = true;
		state->out->flow |= FMT_SET_COLOR;
		goto known;
	};

	if (str_eq_lit(state->peek, "-C") || str_eq_lit(state->peek, "--no-color")) {
		state     ->color = false;
		state->out->flow &= ~FMT_SET_COLOR;
		goto known;
	};

	return ARGS_UNKNOWN;

known:
	arg_skip(&state->args);
	return ARGS_KNOWN;
};

