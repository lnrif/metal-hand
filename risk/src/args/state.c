#include "args/args.h"

ArgsState args_state_init(FmtBump * out, u8 const * const * ptr, u32 count) {
	ASSERT_DEBUG(out != NULL, "expected [out]");
	ASSERT_DEBUG(ptr != NULL, "expected [ptr]");
	ASSERT_DEBUG(count >= 1, "expected at least 1 argument - path to compiler");

	ArgsState state = {
		.out = out, .cmd = STR_NONE,
		.compiler = STR_NONE, .peek = STR_NONE,
		.ptr = ptr, .len = count, .at = 0,
		.color = TRUE, .help = FALSE,
	};

	state.compiler = STR_Z(state.ptr[0]);
	args_state_skip(&state);

	return state;
};

void args_state_skip(ArgsState * state) {
	if (state->at == state->len - 1) {
		state->at = state->len;
		state->peek = STR("");
		return;
	};

	state->at += 1;
	state->peek = STR_Z(state->ptr[state->at]);
};

bool args_state_eof(ArgsState const * state) {
	return state->at == state->len;
};

ArgsResult args_skip_flag_try(ArgsState * state) {
	if (str_prefix(state->peek, STR("-/")) || str_prefix(state->peek, STR("--/"))) {
		args_state_skip(state);
		return ARGS_KNOWN;
	};
	return ARGS_UNKNOWN;
};

bool args_state_radical_ex(CallLoc call, ArgsState * state) {
	if (str_prefix(state->peek, STR("-"))) args_unknown_flag_ex(call,state);
	else                                   args_unexpected_arg_ex(call, state);
	return FALSE;
};


