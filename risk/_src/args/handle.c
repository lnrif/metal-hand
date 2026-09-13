#include "args/args.h"

bool args_handle(
	FmtBump * out,
	ArgsCmd * cmd,
	u8 const * const * ptr,
	u32 count
) {
	ASSERT_DEBUG(out != NULL, "expected [out]");
	ASSERT_DEBUG(cmd != NULL, "expected [cmd]");
	ASSERT_DEBUG(count > 0 && ptr != NULL, "invalid arguments");

	*cmd = (ArgsCmd){
		.as = {0},
		.color = cmd->color,
		.kind = ARGS_CMD_NONE,
	};

	ArgsState state = args_state_init(out, ptr, count);

	if (count == 1 || str_eq_lit(state.peek, "-h") || str_eq_lit(state.peek, "--help")) {
		return args_help_generic(&state);
	};

	if (str_eq_lit(state.peek, "h") || str_eq_lit(state.peek, "help")) {
		args_state_skip(&state); if (args_state_eof(&state)) return args_help(&state);
		state.help = TRUE;
	};

	if (str_eq_lit(state.peek, "b") || str_eq_lit(state.peek, "build")) {
		return args_build(&state, cmd);
	};

	if (state.help) return args_help(&state);

	args_unknown_command(&state);
	return FALSE;
};

