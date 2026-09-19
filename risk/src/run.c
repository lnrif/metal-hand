#include "std/proc/exit.h"
#include "std/run/run.h"
#include "std/mem/page.h"
#include "std/fmt/core.h"

void run(Run run) { UNUSED(run);
	// |================================================================================================|
	// |> setup console                                                                                 |

	Stream _out = stream_output(); Stream * out = &_out;
	if (!stream_is_terminal(out)) proc_exit(255);

	if (!stream_enable_ansi(out)) {
		stream_write_lit(out, "[risk:fatal] failed to enable ANSI in console\n\n");
		proc_exit(255);
	};

	if (!term_enable_utf8()) {
		stream_write_lit(out, "[risk:fatal] failed to enable UTF-8 in console\n\n");
		proc_exit(255);
	};

	// |================================================================================================|
	// |> setup fmt out                                                                                 |

	Pages fmt_pages = pages_reserve(MB(16));
	if (!fmt_pages.is_valid) {
		stream_write_lit(out,
			ANSI_BOLD ANSI_RED "[risk:fatal] "
			ANSI_WHITE "failed to reserve pages for formatter" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	Fmt _fmt = fmt_init(pages_pinned(&fmt_pages), 0b01); Fmt * fmt = &_fmt;

	Pages tmp_pages = pages_reserve(MB(16));
	if (!tmp_pages.is_valid) {
		stream_write_lit(out,
			ANSI_BOLD ANSI_RED "[risk:fatal] "
			ANSI_WHITE "failed to reserve pages for formatter" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	Fmt _tmp = fmt_init(pages_pinned(&tmp_pages), 0b01); Fmt * tmp = &_tmp;

	// |================================================================================================|
	// |> setup paths pool                                                                              |

	// Pages paths_pages = pages_reserve(MB(16));
	// if (!fmt_pages.is_valid) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[risk:fatal] "
	// 		ANSI_WHITE "failed to reserve pages for paths pool" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(255);
	// };
	//
	// Fmt _paths = fmt_init(pages_pinned(&paths_pages), 0); Fmt * paths = &_paths;

	// |================================================================================================|
	// |> hello                                                                                         |

	// fmt_lit(fmt, ANSI_BOLD ANSI_MAGENTA "\n" "Hello, Risky!" "\n" "\n" ANSI_RESET);
	// if (!fmt_flush_stream(fmt, out)) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[risk:error] "
	// 		ANSI_WHITE "???" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(1);
	// };

	// fmt_lit_z(paths, "build");
	// StrMut const build_dir = paths->last;
	// fmt_lit_z(paths, "build/main-x86-64-linux.asm");
	// StrMut const build_file = paths->last;

	// fmt_lit(fmt,
	// 	"format ELF64 executable 3"                                 "\n"
	// 	"entry start"                                               "\n"
	// 	""                                                          "\n"
	// 	"segment readable executable"                               "\n"
	// 	"start:"                                                    "\n"
	// 	"\t" "; write(...)"                                         "\n"
	// 	"\t" "mov rax, 1"                                           "\n"
	// 	"\t" "mov rdi, 1"                                           "\n"
	// 	"\t" "mov rsi, msg"                                         "\n"
	// 	"\t" "mov rdx, len"                                         "\n"
	// 	"\t" "syscall"                                              "\n"
	// 	"\t" "; exit(0)"                                            "\n"
	// 	"\t" "mov rax, 60"                                          "\n"
	// 	"\t" "mov rdi, 0"                                           "\n"
	// 	"\t" "syscall"                                              "\n"
	// 	""                                                          "\n"
	// 	"segment readable"                                          "\n"
	// 	"; message"                                                 "\n"
	// 	"msg: db 27, \"[1;35m\", \"Hello, Risky!\", 27, \"[m\", 10" "\n"
	// 	"len = $ - msg"                                             "\n"
	// 	""                                                          "\n"
	// );

	// if (!fs_dir_create(build_dir.raw)) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[risk:error] "
	// 		ANSI_WHITE "cannot create build folder" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(1);
	// };

	// if (!fmt_flush_file(fmt, build_file.raw)) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[risk:error] "
	// 		ANSI_WHITE "cannot write to build file" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(1);
	// };

	FMT(tmp,
		FMT_LIT("w = "),
		FMT_U64(69, .opt = FMT_N_BIN, .digits = 8),
	); Str a0 = tmp->last.str;

	FMT(tmp,
		FMT_LIT("w = "),
		FMT_U64(69, .opt = FMT_N_OCT, .digits = 8),
	); Str a1 = tmp->last.str;

	FMT(tmp,
		FMT_LIT("w = "),
		FMT_U64(69, .opt = FMT_N_DEC, .digits = 8),
	); Str a2 = tmp->last.str;

	FMT(tmp,
		FMT_LIT("w = "),
		FMT_U64(69, .opt = FMT_N_HEX, .digits = 8),
	); Str a3 = tmp->last.str;

	// u32 const M = (u32)MAX(a0.len, MAX(a1.len, MAX(a2.len, a3.len)));
	u32 const M = (u32)MAX_U64(a0.len, a1.len, a2.len, a3.len);

	FMT(fmt,
		FMT_LIT("| "), FMT_STR(a0, .width = M), FMT_LIT(" |\n"),
		FMT_LIT("| "), FMT_STR(a1, .width = M), FMT_LIT(" |\n"),
		FMT_LIT("| "), FMT_STR(a2, .width = M), FMT_LIT(" |\n"),
		FMT_LIT("| "), FMT_STR(a3, .width = M), FMT_LIT(" |\n"),
	);

	fmt_reset(tmp);
	fmt_flush_stream(fmt, out);

	// stream_write_arr(out,
	// 	SL(ANSI_BOLD ANSI_CYAN " --> "),
	// 	SS(build_file.str),
	// 	SL(ANSI_RESET "\n"),
	// );

	proc_exit(0);
};

