#include "args/args.h"

ArgsResult args_files_try(ArgsState * state, ArgsFiles * files) {
	if (str_prefix(state->peek, STR("-"))) return ARGS_UNKNOWN;

	if (files->input.ptr == NULL) {
		files->input = state->peek;
		goto known;
	};

	if (files->output.ptr == NULL) {
		files->output = state->peek;
		goto known;
	};

	return ARGS_UNKNOWN;

known:
	args_state_skip(state);
	return ARGS_KNOWN;
};

bool args_files_finish(ArgsState * state, ArgsFiles * files, bool diag) {
	if (files->input.ptr == NULL || files->input.len == 0) {
		if (diag) args_expected_file(state, STR("<input.rk>"));
		return FALSE;
	};

	if (files->output.ptr != NULL && files->output.len != 0) return TRUE;

	u8 const * ptr = files->input.ptr;
	for (usz i = files->input.len;;) {
		i -= 1;

		if (ptr[i] == '.') {
			files->output = files->input;
			files->output.len = i;
			return TRUE;
		};

		if (i == 0) goto default_output;
	};

default_output:
	files->output = STR("output");
	return TRUE;
};

bool args_expected_file_ex(CallLoc call, ArgsState * state, Str arg) {
	return fmt_write(state->out,
		// message
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE),  FMT_LIT("expected "),
		FMT_COLOR(FMT_CYAN),   FMT_STR(arg), FMT_LIT(" "),
		FMT_COLOR(FMT_WHITE),  FMT_LIT("for "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("'"), FMT_STR(state->cmd), FMT_LIT("'"),
		FMT_COLOR(FMT_WHITE),  FMT_LIT(" command"), FMT_LIT("\n"),
		// help
		FMT_COLOR(FMT_RED), FMT_LIT("| "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("help: "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("check "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("'"),
		FMT_STR(state->compiler), FMT_LIT(" "), FMT_STR(state->cmd), FMT_LIT(" --help"),
		FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

