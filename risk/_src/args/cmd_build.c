#include "args/args.h"
#include "trc/assert.h"

static bool args_build_help(ArgsState * state, ArgsFiles * files, ArgsMem * mem) {
	args_files_finish(state, files, FALSE);

	Str beg, end, input;
	if (files->input.ptr == NULL) {
		beg = STR(""); end = STR("");
		input = STR("\x1b[" FMT_RED "m");
	} else {
		beg = STR(" = '"); end = STR("'");
		input = STR("\x1b[" FMT_CYAN "m");
	};

	// Str const release = state->release ? STR("enabled (true)") : STR("disabled (false, debug mode)");
	Str const release = FALSE ? STR("enabled (true)") : STR("disabled (false, debug mode)");

	(void)fmt_write(state->out,
		FMT_COLOR(FMT_RESET ";" FMT_BOLD ";" FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("b, build"), FMT_COLOR(FMT_CYAN), FMT_LIT(" <input.rk>"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("command for building a program from source code:"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("1) read file "), FMT_COLOR(FMT_CYAN),
			FMT_LIT("<input.rk>"), FMT_STR(beg), FMT_STR(files->input), FMT_STR(end),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("2) parse, build AST, type and others checks..."),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("3) if not errors - compile, otherwise print diagnostic and stop"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("4) programs put to "), FMT_COLOR(FMT_CYAN),
			FMT_LIT("[output]"), FMT_STR(beg), FMT_STR(files->output), FMT_STR(end),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_COLOR(FMT_RED), FMT_LIT("[!]"),
		FMT_COLOR(FMT_GREY), FMT_LIT(" if file "),
		FMT_COLOR(FMT_CYAN), FMT_LIT("[output]"),
		FMT_COLOR(FMT_GREY), FMT_LIT(" exist, then truncate it"),
	);

	(void)fmt_write(state->out,
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("|\n|> "),
		FMT_COLOR_DYN(input), FMT_LIT("<input.rk>"), FMT_STR(beg), FMT_STR(files->input), FMT_STR(end),
	);

	(void)fmt_write(state->out,
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_CYAN), FMT_LIT("[output]"), FMT_STR(beg), FMT_STR(files->output), FMT_STR(end),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("optinal output path, by default <input>"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("if input file does not have extention, then \"output\""),
	);

	(void)fmt_write(state->out,
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("[--release] = "), FMT_STR(release),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("enables release build mode, makes executable faster"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("but building can took more time and memory"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("-m, --mem-limit"),
		FMT_COLOR(FMT_CYAN), FMT_LIT(" <memory> = "), FMT_MEM(mem->max),
		FMT_COLOR(FMT_GREY), FMT_LIT(" (for example 0, 64KiB, 16MiB, 4GiB)"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("limit maximal memory usage"),
	);

	(void)fmt_write(state->out,
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("|> "),
		FMT_COLOR(FMT_ORANGE), FMT_LIT("[-h, --help]"),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LIT("shows this message"),

		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LINE, FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	);

	return TRUE;
};

bool args_build(ArgsState * state, ArgsCmd * cmd) {
	ASSERT_DEBUG(str_eq_lit(state->peek, "b") || str_eq_lit(state->peek, "build"), "expected command");
	args_state_skip(state); state->cmd = STR("build");

	ArgsFiles files = {.input = STR_NONE, .output = STR_NONE};
	ArgsMem mem; {
		usz const mem_min = MAX(mem_alloc_align(), mem_commit_align() * 16);
		mem = (ArgsMem){.min = mem_min, .max = MAX(mem_min, MB(256))};
	};

	while (!args_state_eof(state)) {
		ARGS_TRY(args_skip_flag_try(state));
		ARGS_TRY(args_common_try(state));
		ARGS_TRY(args_files_try(state, &files));
		ARGS_TRY(args_mem_try(state, &mem));
		return args_state_radical(state);
	};

	if (state->help) return args_build_help(state, &files, &mem);
	if (!args_files_finish(state, &files, TRUE)) return FALSE;

	*cmd = (ArgsCmd){
		.as.build = (ArgsCmdBuild){
			.input = files.input,
			.output = files.output,
			.mem = mem.max,
		},
		.color = state->color,
		.kind = ARGS_CMD_BUILD,
	};

	return TRUE;
};


