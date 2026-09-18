#include "args/args.h"

bool args_unknown_command_ex(CallLoc call, ArgsState * state) {
	return fmt_write(state->out,
		// main message
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("unknown command "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"),
		FMT_LIT("\n"),
		// location
		FMT_COLOR(FMT_RED), FMT_LIT("| "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("help: "), FMT_COLOR(FMT_WHITE),
		FMT_LIT("check "), FMT_COLOR(FMT_GREEN), FMT_LIT("'"),
			 FMT_STR(state->compiler), FMT_LIT(" help"),
		FMT_LIT("'"),
		FMT_COLOR(FMT_WHITE), FMT_LIT(" for commands list"),
		FMT_LINE,
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

bool args_unknown_flag_ex(CallLoc call, ArgsState * state) {
	return fmt_write(state->out,
		// message
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "), FMT_COLOR(FMT_WHITE), FMT_LIT("unknown flag "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"),
		FMT_COLOR(FMT_WHITE), FMT_LIT(" for command "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(state->cmd), FMT_LIT("'"),
		FMT_LIT("\n"),
		// help
		FMT_COLOR(FMT_RED), FMT_LIT("| "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("help: "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("check "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("'"),
		FMT_STR(state->compiler), FMT_LIT(" "), FMT_STR(state->cmd), FMT_LIT(" --help"),
		FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

bool args_unexpected_arg_ex(CallLoc call, ArgsState * state) {
	return fmt_write(state->out,
		// message
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "), FMT_COLOR(FMT_WHITE), FMT_LIT("unexpected argument "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"),
		FMT_COLOR(FMT_WHITE), FMT_LIT(" for command "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(state->cmd), FMT_LIT("'"),
		FMT_LIT("\n"),
		// help
		FMT_COLOR(FMT_RED), FMT_LIT("| "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("help: "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("check "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("'"),
		FMT_STR(state->compiler), FMT_LIT(" "), FMT_STR(state->cmd), FMT_LIT(" --help"),
		FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};


