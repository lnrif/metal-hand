#include "std/proc/exit.h"
#include "std/run/run.h"
#include "std/mem/page.h"
#include "std/fmt/core.h"
#include "risk/src/args/args.h"

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

	Fmt _fmt = fmt_init(pages_pinned(&fmt_pages), FMT_SET_TEXT | FMT_SET_COLOR); Fmt * fmt = &_fmt;

	// Pages tmp_pages = pages_reserve(MB(16));
	// if (!tmp_pages.is_valid) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[risk:fatal] "
	// 		ANSI_WHITE "failed to reserve pages for formatter" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(255);
	// };
	//
	// Fmt _tmp = fmt_init(pages_pinned(&tmp_pages), 0b01); Fmt * tmp = &_tmp;

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

	// ArgsCmd cmd = {0}; {
	// 	if (!args_handle(fmt, cmd, (u8 const * const *)argv, (u32)argc)) goto err;
	// 	fmt_flush_stream(fmt, out);
	// };

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

	FMT(fmt, FMT_LIT("\n"), FMT_BOLD);


	// FMT(fmt,
	// 	FMT_RED, FMT_LIT("mut"),
	// 	FMT_BRIGHT_BLACK, FMT_LIT("'"),
	// 	FMT_YELLOW, FMT_LIT("custom"),
	// 	FMT_LIT(" "),
	// 	FMT_BRIGHT_BLACK, FMT_LIT("{"),
	// 	FMT_BLUE, FMT_LIT("a"),
	// 	FMT_BRIGHT_BLACK, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_LIT("b"),
	// 	FMT_BRIGHT_BLACK, FMT_LIT("}"),
	// 	FMT_YELLOW, FMT_LIT(" := "),
	// 	FMT_BRIGHT_BLACK, FMT_LIT("{"),
	// 	FMT_MAGENTA, FMT_LIT("0"),
	// 	FMT_BRIGHT_BLACK, FMT_LIT(", "),
	// 	FMT_MAGENTA, FMT_LIT("1"),
	// 	FMT_BRIGHT_BLACK, FMT_LIT("}"),
	// 	FMT_BRIGHT_BLACK, FMT_LIT(";"),
	// 	FMT_LIT("\n\n"),
	// );

	// x' := 0; for 1..=0 do x += 1;
	// mut x := 0; for 1..=0 :: x += 1;

	FMT(fmt, FMT_RESET);

	//
	// mut x: u64 = 0;
	// x: u64 = 0;

	// fmt_reset(tmp);

	// stream_write_arr(out,
	// 	SL(ANSI_BOLD ANSI_CYAN " --> "),
	// 	SS(build_file.str),
	// 	SL(ANSI_RESET "\n"),
	// );

ok:
	fmt_flush_stream(fmt, out);
	proc_exit(0);

err:
	fmt_flush_stream(fmt, out);
	proc_exit(1);
};

