#include "risk/src/args/args.h"
#include "std/run/arg.h"

b8 args_handle(Fmt * out, ArgsCmd * cmd, Args args) {
	ASSERT_PTR(out);
	ASSERT_PTR(cmd);

	*cmd = (ArgsCmd){
		.as = {0},
		.color = cmd->color,
		.kind = ARGS_CMD_NONE,
	};

	ArgsState state = args_state_init(out, args);
	// Str test = arg_next(&state.args);

	if (str_nil(state.peek) || str_eq(state.peek, S("-h")) || str_eq(state.peek, S("--help"))) {
		return args_help_generic(&state);
	};

	if (str_eq(state.peek, S("h")) || str_eq(state.peek, S("help"))) {
		arg_skip(&state.args); if (str_nil(state.peek)) return args_help(&state);
		state.help = true;
	};

	if (str_eq(state.peek, S("b")) || str_eq(state.peek, S("build"))) {
		return args_build(&state, cmd);
	};

	if (state.help) return args_help(&state);

	args_unknown_command(&state);
	return false;
};

