#include "risk/src/args/args.h"
#include "std/mem/page.h"
// #include "std/"

static b8 args_build_help(ArgsState * state, ArgsFiles * files, ArgsMem * mem) {
	args_files_finish(state, files, false);

	Str beg, end, input;
	if (files->input.ptr == 0) {
		beg = S(""); end = S("");
		input = S("\x1b[" ANSI_RED "m");
	} else {
		beg = S(" = '"); end = S("'");
		input = S("\x1b[" ANSI_CYAN "m");
	};

	// Str const release = state->release ? S("enabled (true)") : S("disabled (false, debug mode)");
	Str const release = false ? S("enabled (true)") : S("disabled (false, debug mode)");

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

	return true;
};

b8 args_build(ArgsState * state, ArgsCmd * cmd) {
	ASSERT(str_eq_lit(state->peek.str, "b") || str_eq_lit(state->peek.str, "build"), "expected command");
	args_state_skip(state); state->cmd = S("build");

	ArgsFiles files = {.input = STR_NIL, .output = STR_NIL};
	ArgsMem mem; {
		u64 const mem_min = MAX(pages_align(), pages_align_atom() * 16);
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
	if (!args_files_finish(state, &files, true)) return false;

	*cmd = (ArgsCmd){
		.as.build = (ArgsCmdBuild){
			.input = files.input,
			.output = files.output,
			.mem = mem.max,
		},
		.color = state->color,
		.kind = ARGS_CMD_BUILD,
	};

	return true;
};


