#include "risk/src/args/args.h"
#include "std/flow/core.h"
#include "std/fmt/core.h"

#define RISK_VERSION "v0.1.0-debug"

b8 args_help_generic(ArgsState * state) {
	return FMT(state->out,
		FMT_BOLD, FMT_RED, FMT_LIT("RISKy COMPILER"),
		FMT_WHITE, FMT_LIT(" " RISK_VERSION), FMT_LIT("\n"),
		FMT_LIT("\n"),

		FMT_GREEN, FMT_LIT("USAGE: "),
			FMT_RED, FMT_STR(state->compiler),
			FMT_ORANGE, FMT_LIT(" [command]"),
		FMT_LIT("\n"),

		FMT_LIT("\n"),

		FMT_GREY, FMT_LIT("|> "),
		FMT_ORANGE, FMT_LIT("h, help"),
		FMT_GREY, FMT_LIT(", -h, --help"),
		FMT_RESET, FMT_BOLD, FMT_CYAN,
			FMT_LIT(" [command] [args...]"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
			FMT_LIT("show all commands / info about [command] and [args...]"),
		FMT_LIT("\n"),

		FMT_LIT("\n"),

		FMT_GREY, FMT_LIT("|> "),
		FMT_ORANGE, FMT_LIT("b, build"),
		FMT_CYAN,
			FMT_LIT(" <input.rk> [output]"),
			FMT_GREEN, FMT_LIT(" [--release]"),
			FMT_ORANGE, FMT_LIT(" [-h, --help]"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
			FMT_LIT("build <input.rk> and put executable <input> | [output]"),
		FMT_LIT("\n"),

		// FMT_GREY, FMT_LIT("|"), FMT_LIT("\n"),

		// ARGS_FMT_COMMAND, FMT_LIT("c, check"),
		// FMT_CYAN,
		// 	FMT_LIT(" <input.rk>"),
		// 	FMT_ORANGE, FMT_LIT(" [-h, --help]"),
		// FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		// 	FMT_LIT("check <input.rk> and print diagnostic"),
		// FMT_LIT("\n"),

		// FMT_GREY, FMT_LIT("| "), FMT_LIT("\n"),

		FMT_GREY, FMT_LIT("|- "),
			FMT_GREEN, FMT_LIT("-m, --mem-limit"),
			FMT_CYAN, FMT_LIT(" <memory>"),
			FMT_GREY, FMT_LIT(" (for example 0, 64KiB, 16MiB, 4GiB)"),
		FMT_LIT("\n"), FMT_GREY,
			FMT_LIT("| limit maximal memory usage"),
		FMT_LIT("\n"),

		FMT_LIT("\n"),

		FMT_LOC_DEBUG(FLOW_LOC),
		FMT_RESET,
	);
};

static b8 args_help_detail(ArgsState * state) {
	return FMT(state->out,
		FMT_BOLD, FMT_GREY,
		FMT_LIT("|> "), FMT_ORANGE,
		FMT_ORANGE, FMT_LIT("h, help"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("command for helping and showing what command will do"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_RED, FMT_STR(state->compiler),
		FMT_ORANGE, FMT_LIT(" build "),
		FMT_CYAN, FMT_LIT("main.rk "),
		FMT_ORANGE, FMT_LIT("--help"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("# or the same"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_RED, FMT_STR(state->compiler),
		FMT_ORANGE, FMT_LIT(" help build "),
		FMT_CYAN, FMT_LIT("main.rk"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "), FMT_LOC_DEBUG(FLOW_LOC),
		FMT_RED, FMT_LIT("  '\n"),
		FMT_RESET,
	);
};

b8 args_help(ArgsState * state) {
	state->cmd = S("help");
	state->help = false;

	while (str_sane(state->peek)) {
		ARGS_TRY(args_skip_flag_try(state));
		ARGS_TRY(args_common_try(state));

		if (str_eq_lit(state->peek, "h") || str_eq_lit(state->peek, "help")) {
			state->help = true;
			arg_skip(&state->args);
			continue;
		};

		return args_state_radical(state);
	};

	if (state->help) return args_help_detail(state);

	return args_help_generic(state);
};

