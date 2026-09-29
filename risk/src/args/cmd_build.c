#include "risk/src/args/args.h"
#include "std/mem/page.h"

static b8 args_build_help(ArgsState * state, ArgsFiles * files, ArgsMem * mem) {
	args_files_finish(state, files, false);

	Str beg, end, input;
	if (files->input.ptr == 0) {
		beg = S(""); end = S("");
		input = S(ANSI_RED);
	} else {
		beg = S(" = '"); end = S("'");
		input = S(ANSI_CYAN);
	};

	// Str const release = state->release ? S("enabled (true)") : S("disabled (false, debug mode)");
	Str const release = false ? S("enabled (true)") : S("disabled (false, debug mode)");

	FMT(state->out,
		FMT_RESET, FMT_BOLD, FMT_GREY, FMT_LIT("|> "),
		FMT_ORANGE, FMT_LIT("b, build"), FMT_CYAN, FMT_LIT(" <input.rk>"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("command for building a program from source code:"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("1) read file "), FMT_CYAN,
			FMT_LIT("<input.rk>"), FMT_STR(beg), FMT_STR(files->input), FMT_STR(end),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("2) parse, build AST, type and others checks..."),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("3) if not errors - compile, otherwise print diagnostic and stop"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("4) programs put to "), FMT_CYAN,
			FMT_LIT("[output]"), FMT_STR(beg), FMT_STR(files->output), FMT_STR(end),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_RED, FMT_LIT("[!]"),
		FMT_GREY, FMT_LIT(" if file "),
		FMT_CYAN, FMT_LIT("[output]"),
		FMT_GREY, FMT_LIT(" exist, then truncate it"),
	);

	FMT(state->out,
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("|\n|> "),
		FMT_COLOR(input), FMT_LIT("<input.rk>"), FMT_STR(beg), FMT_STR(files->input), FMT_STR(end),
	);

	FMT(state->out,
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("|> "),
		FMT_CYAN, FMT_LIT("[output]"), FMT_STR(beg), FMT_STR(files->output), FMT_STR(end),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("optinal output path, by default <input>"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("if input file does not have extention, then \"output\""),
	);

	FMT(state->out,
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("|> "),
		FMT_GREEN, FMT_LIT("[--release] = "), FMT_STR(release),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("enables release build mode, makes executable faster"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("but building can took more time and memory"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("|> "),
		FMT_GREEN, FMT_LIT("-m, --mem-limit"),
		FMT_CYAN, FMT_LIT(" <memory> = "), FMT_MEM(mem->max),
		FMT_GREY, FMT_LIT(" (for example 64KiB, 16MiB, 4GiB)"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("limit maximal memory usage"),
	);

	FMT(state->out,
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("|> "),
		FMT_ORANGE, FMT_LIT("[-h, --help]"),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("shows this message"),

		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LIT("\n"), FMT_GREY, FMT_LIT("| "),
		FMT_LOC_DEBUG(FLOW_LOC),
		FMT_GREY, FMT_LIT("'\n"),
		FMT_RESET,
	);

	return true;
};

typedef struct {
	ArgsBuildLex lex;
	b8 ast;
} ArgsBuildFlags;

ArgsResult args_build_flags_try(ArgsState * state, ArgsBuildFlags * flags) {
	if (str_eq_lit(state->peek, "--lex")) {
		flags->lex = ARGS_BUILD_LEX_SHORT;
		goto known;
	};

	if (str_eq_lit(state->peek, "--lex-full")) {
		flags->lex = ARGS_BUILD_LEX_FULL;
		goto known;
	};

	if (str_eq_lit(state->peek, "--ast")) {
		flags->ast = true;
		goto known;
	};

	return ARGS_UNKNOWN;

known:
	arg_skip(&state->args);
	return ARGS_KNOWN;
};

b8 args_build(ArgsState * state, ArgsCmd * cmd) {
	ASSERT(str_eq(state->peek, S("b")) || str_eq(state->peek, S("build")), "expected command");
	arg_skip(&state->args); state->cmd = S("build");

	ArgsFiles files = {.input = STR_NIL, .output = STR_NIL};
	ArgsMem mem; {
		u64 const mem_min = MAX(pages_align(), pages_align_atom() * 16);
		mem = (ArgsMem){.min = mem_min, .max = MAX(mem_min, MB(256))};
	};

	ArgsBuildFlags flags = (ArgsBuildFlags){.lex = ARGS_BUILD_LEX_NO, .ast = false};

	while (str_sane(state->peek)) {
		ARGS_TRY(args_skip_flag_try(state));
		ARGS_TRY(args_common_try(state));
		ARGS_TRY(args_files_try(state, &files));
		ARGS_TRY(args_mem_try(state, &mem));
		ARGS_TRY(args_build_flags_try(state, &flags));
		return args_state_radical(state);
	};

	if (state->help) return args_build_help(state, &files, &mem);
	if (!args_files_finish(state, &files, true)) return false;

	*cmd = (ArgsCmd){
		.as.build = (ArgsCmdBuild){
			.input = files.input, .output = files.output,
			.lex = flags.lex, .ast = flags.ast,
			.mem = mem.max,
		},
		.color = state->color,
		.kind = ARGS_CMD_BUILD,
	};

	return true;
};


