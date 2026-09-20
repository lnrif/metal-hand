#include "risk/src/args/args.h"
#include "std/flow/core.h"

b8 args_unknown_command_ex(FlowLoc flow, ArgsState * state) {
	return FMT(state->out,
		// main message
		FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
		FMT_WHITE, FMT_LIT("unknown command "),
		FMT_RED, FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"),
		FMT_LIT("\n"),
		// location
		FMT_RED, FMT_LIT("  | "),
		FMT_GREEN, FMT_LIT("help: "), FMT_WHITE,
		FMT_LIT("check "), FMT_GREEN, FMT_LIT("'"),
			 FMT_STR(state->compiler), FMT_LIT(" help"),
		FMT_LIT("'"),
		FMT_WHITE, FMT_LIT(" for commands list"),
		FMT_LIT("\n"),
		// debug
		FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
		FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
		FMT_RESET,
	);
};

b8 args_unknown_flag_ex(FlowLoc flow, ArgsState * state) {
	return FMT(state->out,
		// message
		FMT_BOLD, FMT_RED, FMT_LIT("[e] "), FMT_WHITE, FMT_LIT("unknown flag "),
		FMT_RED, FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"),
		FMT_WHITE, FMT_LIT(" for command "),
		FMT_RED, FMT_LIT("'"), FMT_STR(state->cmd), FMT_LIT("'"),
		FMT_LIT("\n"),
		// help
		FMT_RED, FMT_LIT("  | "),
		FMT_GREEN, FMT_LIT("help: "),
		FMT_WHITE, FMT_LIT("check "),
		FMT_GREEN, FMT_LIT("'"),
		FMT_STR(state->compiler), FMT_LIT(" "), FMT_STR(state->cmd), FMT_LIT(" --help"),
		FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
		FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
		FMT_RESET,
	);
};

b8 args_unexpected_arg_ex(FlowLoc flow, ArgsState * state) {
	return FMT(state->out,
		// message
		FMT_BOLD, FMT_RED, FMT_LIT("[e] "), FMT_WHITE, FMT_LIT("unexpected argument "),
		FMT_RED, FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"),
		FMT_WHITE, FMT_LIT(" for command "),
		FMT_RED, FMT_LIT("'"), FMT_STR(state->cmd), FMT_LIT("'"),
		FMT_LIT("\n"),
		// help
		FMT_RED, FMT_LIT("  | "),
		FMT_GREEN, FMT_LIT("help: "),
		FMT_WHITE, FMT_LIT("check "),
		FMT_GREEN, FMT_LIT("'"),
		FMT_STR(state->compiler), FMT_LIT(" "), FMT_STR(state->cmd), FMT_LIT(" --help"),
		FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
		FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
		FMT_RESET,
	);
};


