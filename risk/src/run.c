#include "std/proc/exit.h"
#include "std/run/run.h"
#include "std/mem/page.h"
#include "std/fmt/core.h"
#include "risk/src/args/args.h"
#include "std/run/arg.h"

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

	// Fmt _tmp = fmt_init(pages_pinned(&tmp_pages), FMT_SET_TEXT | FMT_SET_COLOR); Fmt * tmp = &_tmp;

	// |================================================================================================|
	// |> setup paths pool                                                                              |

	// Pages paths_pages = pages_reserve(MB(16));
	// if (!paths_pages.is_valid) {
	// 	stream_write_lit(out,
	// 		ANSI_BOLD ANSI_RED "[risk:fatal] "
	// 		ANSI_WHITE "failed to reserve pages for paths pool" "\n"
	// 		ANSI_RESET
	// 	);
	// 	proc_exit(255);
	// };

	// Fmt _paths = fmt_init(pages_pinned(&paths_pages), 0); Fmt * paths = &_paths;

	// |================================================================================================|
	// |> hello                                                                                         |


	// ArgsCmd cmd = {0}; {
	// 	Args args = arg_init(&run);
	// 	if (!args_handle(fmt, &cmd, args)) goto err;
	// 	fmt_flush_stream(fmt, out);
	// };

	u8 const digits = 2;
	FMT(fmt,
		FMT_BOLD,
		FMT_YELLOW, FMT_LIT("[["),
		FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.457, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.568, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	);

	FMT(fmt,
		FMT_BOLD,
		FMT_YELLOW, FMT_LIT(" ["),
		FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.568, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	);

	FMT(fmt,
		FMT_BOLD,
		FMT_YELLOW, FMT_LIT(" ["),
		FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.568, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	);

	FMT(fmt,
		FMT_BOLD,
		FMT_YELLOW, FMT_LIT(" ["),
		FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.568, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	);

	FMT(fmt,
		FMT_BOLD,
		FMT_YELLOW, FMT_LIT(" ["),
		FMT_BLUE, FMT_F64(-0.124, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT(", "),
		FMT_RED, FMT_F64(+0.568, .digits = digits, .opt = FMT_N_SIGN),
		FMT_YELLOW, FMT_LIT("]]"), FMT_LIT("\n"),
	);

// ok:
	fmt_flush_stream(fmt, out);
	proc_exit(0);

// err:
	fmt_flush_stream(fmt, out);
	proc_exit(1);
};

