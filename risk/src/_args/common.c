#include "args/args.h"

ArgsResult args_common_try(ArgsState * state) {
	if (str_eq_lit(state->peek, "-h") || str_eq_lit(state->peek, "--help")) {
		state->help = TRUE;
		goto known;
	};

	if (str_eq_lit(state->peek, "-c") || str_eq_lit(state->peek, "--color")) {
		state     ->color = TRUE;
		state->out->color = TRUE;
		goto known;
	};

	if (str_eq_lit(state->peek, "-C") || str_eq_lit(state->peek, "--no-color")) {
		state     ->color = FALSE;
		state->out->color = FALSE;
		goto known;
	};

	return ARGS_UNKNOWN;

known:
	args_state_skip(state);
	return ARGS_KNOWN;
};

