#include "risk/src/args/args.h"
#include "std/flow/core.h"
#include "std/str/core.h"

ArgsState args_state_init(Fmt * out, Args args) {
	ASSERT_PTR(out);

	ArgsState state = {
		.out = out, .cmd = STR_NIL,
		.compiler = STR_NIL, .args = args,
		.color = true, .help = false,
	};

	state.compiler = arg_next(&state.args);
	ASSERT(str_sane(state.compiler), "expected at least 1 argument - path to compiler");

	return state;
};

ArgsResult args_skip_flag_try(ArgsState * state) {
	if (!str_beg(state->peek, S("-/")) && !str_beg(state->peek, S("--/"))) return ARGS_UNKNOWN;
	arg_next(&state->args);
	return ARGS_KNOWN;
};

b8 args_state_radical_ex(FlowLoc loc, ArgsState * state) {
	if (str_beg(state->peek, S("-"))) args_unknown_flag_ex  (loc, state);
	else                              args_unexpected_arg_ex(loc, state);
	return false;
};

