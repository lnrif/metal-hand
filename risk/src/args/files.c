#include "risk/src/args/args.h"
#include "std/fmt/core.h"

ArgsResult args_files_try(ArgsState * state, ArgsFiles * files) {
	if (str_beg(state->peek, S("-"))) return ARGS_UNKNOWN;

	if (str_nil(files->input)) {
		files->input = state->peek;
		goto known;
	};

	if (str_nil(files->output)) {
		files->output = state->peek;
		goto known;
	};

	return ARGS_UNKNOWN;

known:
	arg_skip(&state->args);
	return ARGS_KNOWN;
};

b8 args_files_finish(ArgsState * state, ArgsFiles * files, b8 diag) {
	if (str_nil(files->input) || str_nil(files->input)) {
		if (diag) args_expected_file(state, S("<input.rk>"));
		return false;
	};

	if (str_sane(files->output) && str_sane(files->output)) return true;

	u8 const * ptr = files->input.raw;
	for (u64 i = files->input.len;;) {
		i -= 1;

		if (ptr[i] == '.') {
			files->output = files->input;
			files->output.len = i;
			return true;
		};

		if (i == 0) goto default_output;
	};

default_output:
	files->output = S("output");
	return true;
};

b8 args_expected_file_ex(FlowLoc flow, ArgsState * state, Str arg) {
	return FMT(state->out,
		// message
		FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
		FMT_WHITE,  FMT_LIT("expected "),
		FMT_CYAN,   FMT_STR(arg), FMT_LIT(" "),
		FMT_WHITE,  FMT_LIT("for "),
		FMT_ORANGE, FMT_LIT("'"), FMT_STR(state->cmd), FMT_LIT("'"),
		FMT_WHITE,  FMT_LIT(" command"), FMT_LIT("\n"),
		// help
		FMT_RED, FMT_LIT("  | "),
		FMT_GREEN, FMT_LIT("help: "),
		FMT_WHITE, FMT_LIT("check "),
		FMT_ORANGE, FMT_LIT("'"),
		FMT_STR(state->compiler), FMT_LIT(" "), FMT_STR(state->cmd), FMT_LIT(" --help"),
		FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_RED, FMT_LIT("  |\n"),
		FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(flow),
		FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC),
		FMT_RED, FMT_LIT("  '\n"),
		FMT_RESET,
	);
};

