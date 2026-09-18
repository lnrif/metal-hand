#include "args/args.h"

#define RISK_VERSION "v0.1.0-debug"

bool args_help_generic(ArgsState * state) {
	return fmt_write(state->out,
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("RISKy COMPILER"),
		FMT_COLOR(FMT_WHITE), FMT_LIT(" " RISK_VERSION), FMT_LINE,
		FMT_LINE,

		FMT_COLOR(FMT_GREEN), FMT_LIT("USAGE: "),
			FMT_COLOR(FMT_RED), FMT_STR(state->compiler),
			FMT_COLOR(FMT_ORANGE), FMT_LIT(" [command]"),
		FMT_LINE,

		FMT_LINE,

		FMT_COLOR(FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("h, help"),
		FMT_COLOR(FMT_DARK), FMT_LIT(", -h, --help"),
		FMT_COLOR(FMT_RESET ";" FMT_BOLD ";" FMT_CYAN),
			FMT_LIT(" [command] [args...]"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
			FMT_LIT("show all commands / info about [command] and [args...]"),
		FMT_LINE,

		FMT_LINE,

		FMT_COLOR(FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("b, build"),
		FMT_COLOR(FMT_CYAN),
			FMT_LIT(" <input.rk> [output]"),
			FMT_COLOR(FMT_GREEN), FMT_LIT(" [--release]"),
			FMT_COLOR(FMT_ORANGE), FMT_LIT(" [-h, --help]"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
			FMT_LIT("build <input.rk> and put executable <input> | [output]"),
		FMT_LINE,

		// FMT_COLOR(FMT_GREY), FMT_LIT("|"), FMT_LINE,

		// ARGS_FMT_COMMAND, FMT_LIT("c, check"),
		// FMT_COLOR(FMT_CYAN),
		// 	FMT_LIT(" <input.rk>"),
		// 	FMT_COLOR(FMT_ORANGE), FMT_LIT(" [-h, --help]"),
		// FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		// 	FMT_LIT("check <input.rk> and print diagnostic"),
		// FMT_LINE,

		// FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LINE,

		FMT_COLOR(FMT_GREY), FMT_LIT("|- "),
			FMT_COLOR(FMT_GREEN), FMT_LIT("-m, --mem-limit"),
			FMT_COLOR(FMT_CYAN), FMT_LIT(" <memory>"),
			FMT_COLOR(FMT_GREY), FMT_LIT(" (for example 0, 64KiB, 16MiB, 4GiB)"),
		FMT_LINE, FMT_COLOR(FMT_GREY),
			FMT_LIT("| limit maximal memory usage"),
		FMT_LINE,

		FMT_LINE,

		FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

static bool args_help_detail(ArgsState * state) {
	return fmt_write(state->out,
		FMT_COLOR(FMT_BOLD ";" FMT_GREY),
		FMT_LIT("|> "), FMT_COLOR(FMT_ORANGE),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("h, help"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("command for helping and showing what command will do"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_COLOR(FMT_RED), FMT_STR(state->compiler),
		FMT_COLOR(FMT_ORANGE), FMT_LIT(" build "),
		FMT_COLOR(FMT_CYAN), FMT_LIT("main.rk "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("--help"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("# or the same"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_COLOR(FMT_RED), FMT_STR(state->compiler),
		FMT_COLOR(FMT_ORANGE), FMT_LIT(" help build "),
		FMT_COLOR(FMT_CYAN), FMT_LIT("main.rk"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

bool args_help(ArgsState * state) {
	state->cmd = STR("help");
	state->help = FALSE;

	while (!args_state_eof(state)) {
		ARGS_TRY(args_skip_flag_try(state));
		ARGS_TRY(args_common_try(state));

		if (str_eq_lit(state->peek, "h") || str_eq_lit(state->peek, "help")) {
			state->help = TRUE;
			args_state_skip(state);
			continue;
		};

		return args_state_radical(state);
	};

	if (state->help) return args_help_detail(state);

	return args_help_generic(state);
};

