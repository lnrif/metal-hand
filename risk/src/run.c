#include "std/mem/page.h"
#include "std/fmt/core.h"
#include "std/fs/core.h"

#include "std/proc/exit.h"
#include "std/run/run.h"
#include "std/run/arg.h"

#include "risk/src/args/args.h"
#include "risk/src/src/core.h"
#include "risk/src/lex/core.h"
#include "risk/src/ast/parser.h"
#include "risk/src/ast/fmt.h"

typedef struct {
	Stream * out; Fmt * fmt;
	Fmt * paths; ArgsCmdBuild args;
} RkBuild;

b8 rk_build(RkBuild build) {
	u8 res = 0;

	Pages file_pages = pages_reserve(MB(16)); if (!file_pages.is_valid) {
		stream_write_lit(build.out,
			ANSI_BOLD ANSI_RED "[f] "
			ANSI_WHITE "failed to reserve pages for formatter" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	FMT(build.paths, FMT_STR(build.args.input), FMT_LIT("\0"));
	StrMut const build_input = build.paths->last;

	FMT(build.paths, FMT_STR(build.args.output), FMT_LIT("\0"));
	StrMut const build_output = build.paths->last;

	FileBuf buf = fs_read_all(pages_pinned(&file_pages), build_input.any); if (!buf.is_valid) {
		FMT(build.fmt,
			FMT_BOLD,
			FMT_RED, FMT_LIT("[E] "),
			FMT_WHITE, FMT_LIT("cannot read file "),
			FMT_CYAN, FMT_LIT("'"), FMT_STR(build_input.str), FMT_LIT("'"), FMT_LIT("\n"),
			FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC), FMT_RESET,
		);
		res = 1; goto exit;
	};

	FMT(build.fmt,
		FMT_BOLD,
		FMT_BLUE, FMT_LIT("[I] "),
		FMT_WHITE, FMT_LIT("read file "),
		FMT_CYAN, FMT_LIT("'"), FMT_STR(build_input.str), FMT_LIT("'"), FMT_LIT("\n"),
		FMT_BLUE, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC),
	);

	u32 DELIM_SIZE = 36;
	if (build.args.lex != ARGS_BUILD_LEX_NO) {
		FMT(build.fmt,
			FMT_LIT("\n"),
			FMT_YELLOW, FMT_REPEAT('=', DELIM_SIZE), FMT_LIT(" LEX "), FMT_REPEAT('=', DELIM_SIZE),
			FMT_LIT("\n\n"),
		);

		Lexer lexer = lex_init(buf.str_z);
		SrcDot dot = SRC_DOT_NIL;

		for (u64 i = 1, j = 1;; i += 1) {
			LexToken const token = lex_next(&lexer);
			if (build.args.lex == ARGS_BUILD_LEX_SHORT && LEX_IS_SPACES(token.tag)) continue;
			j += 1;

			Str const lexeme = STR(&buf.raw[token.pos], token.len);
			dot = src_dot(buf.str, dot, token.pos);

			FMT(build.fmt,
				FMT_BLUE, FMT_LIT("["), FMT_U64(i, .digits = 3), FMT_LIT("] "),
				FMT_YELLOW, FMT_STR(lex_name(token.tag), .width = 12),
				FMT_WHITE, FMT_LIT(" | "),
				FMT_GREEN, FMT_STR(lexeme, .width = 24, .opt = FMT_LHS | FMT_S_QUOTES | FMT_S_ESCAPE),
				FMT_WHITE, FMT_LIT(" | "),
				FMT_CYAN, FMT_STR(build_input.str),
				FMT_LIT(":"), FMT_U64(dot.row),
				FMT_LIT(":"), FMT_U64(dot.col),
				FMT_LIT("\n"),
			);

			if (token.tag == LEX_T_EOF) break;
			if (j % 16 == 0) FMT(build.fmt, FMT_LIT("\n"));
		};

		FMT(build.fmt,
			FMT_LIT("\n"),
			FMT_YELLOW, FMT_REPEAT('=', DELIM_SIZE * 2 + S(" LEX ").len),
			FMT_LIT("\n\n"),
		);
	};

	Pages ast_pages = pages_reserve(GB(1)); if (!file_pages.is_valid) {
		stream_write_lit(build.out,
			ANSI_BOLD ANSI_RED "[f] "
			ANSI_WHITE "failed to reserve pages for formatter" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	Parser parser = parser_init((Src){.str = buf.str, .path = build_input.str}, build.fmt, &ast_pages);
	Ast ast = parser_parse(&parser);

	if (build.args.ast) ast_fmt(build.fmt, &ast, ast.root);

	FMT(build.fmt,
		FMT_BLUE, FMT_LIT("[I] "),
		FMT_WHITE, FMT_LIT("write file "),
		FMT_CYAN, FMT_LIT("'"), FMT_STR(build_output.str), FMT_LIT("'"), FMT_LIT("\n"),
		FMT_BLUE, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC), FMT_RESET,
	);

exit:
	fmt_flush_stream(build.fmt, build.out);
	proc_exit(res);
};

void run(Run run) { UNUSED(run);
	// |================================================================================================|
	// |> setup console                                                                                 |

	Stream _out = stream_output(); Stream * out = &_out;
	if (!stream_is_terminal(out)) proc_exit(255);

	if (!stream_enable_ansi(out)) {
		stream_write_lit(out, "[F] failed to enable ANSI in console\n\n");
		proc_exit(255);
	};

	if (!term_enable_utf8()) {
		stream_write_lit(out, "[F] failed to enable UTF-8 in console\n\n");
		proc_exit(255);
	};

	// |================================================================================================|
	// |> setup fmt out                                                                                 |

	Pages fmt_pages = pages_reserve(MB(16));
	if (!fmt_pages.is_valid) {
		stream_write_lit(out,
			ANSI_BOLD ANSI_RED "[F] "
			ANSI_WHITE "failed to reserve pages for formatter" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	Fmt _fmt = fmt_init(pages_pinned(&fmt_pages), FMT_SET_TEXT | FMT_SET_COLOR); Fmt * fmt = &_fmt;

	// Pages tmp_pages = pages_reserve(MB(16));
	// if (!tmp_pages.is_valid) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[f] "
	// 		ANSI_WHITE "failed to reserve pages for formatter" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(255);
	// };

	// Fmt _tmp = fmt_init(pages_pinned(&tmp_pages), FMT_SET_TEXT | FMT_SET_COLOR); Fmt * tmp = &_tmp;

	// |================================================================================================|
	// |> setup paths pool                                                                              |

	Pages paths_pages = pages_reserve(MB(16));
	if (!paths_pages.is_valid) {
		stream_write_lit(out,
			ANSI_BOLD ANSI_RED "[F] "
			ANSI_WHITE "failed to reserve pages for paths pool" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	Fmt _paths = fmt_init(pages_pinned(&paths_pages), FMT_SET_TEXT); Fmt * paths = &_paths;

	// |================================================================================================|
	// |> hello                                                                                         |

	ArgsCmd cmd = {0}; {
		Args args = arg_init(&run);
		if (!args_handle(fmt, &cmd, args)) goto err;
		fmt_flush_stream(fmt, out);
	};

	switch (cmd.kind) {
		case ARGS_CMD_NONE: proc_exit(0); break;

		case ARGS_CMD_BUILD: {
			rk_build((RkBuild){.out = out, .fmt = fmt, .paths = paths, .args = cmd.as.build});
		} break;

		case ARGS_CMD_RUN: {
			FMT(fmt,
				FMT_BOLD,
				FMT_RED, FMT_LIT("[E] "),
				FMT_WHITE, FMT_LIT("command 'run' is not implemented, sorry"), FMT_LIT("\n"),
				FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC),
				FMT_RESET,
			);
		} break;

		default: PANIC("invalid kind [cmd.kind]");
	};

// ok:
	fmt_flush_stream(fmt, out);
	proc_exit(0);

err:
	fmt_flush_stream(fmt, out);
	proc_exit(1);
};

