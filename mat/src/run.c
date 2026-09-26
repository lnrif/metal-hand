#include "std/core.h"
#include "std/mem/reg.h"
#include "std/proc/exit.h"
#include "std/run/run.h"

#include "std/rnd/core.h"

#include "std/mem/page.h"
#include "std/fmt/core.h"

#include "mat/src/mat.h"

// b8 mat_alloc(Mat * mat, RegMan man, u32 row, u32 col) {
//
// };
//
// b8 lin_init(Lin * lin, RegMan man, Bind x, u32 out) {
// 	u64 len = 0;
// 	Reg reg = reg_upd_arr(man, (f32*)0, 0, len, REG_DIR_UP);
// 	if (!reg.is_valid) return false;
//
// 	f32 * ptr = reg.any;
//
//
//
// 	return true;
// };

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

	// |================================================================================================|
	// |> ...                                                                                           |

	Pages mat_pages = pages_reserve(MB(16));
	if (!mat_pages.is_valid) {
		stream_write_lit(out,
			ANSI_BOLD ANSI_RED "[fatal] "
			ANSI_WHITE "failed to reserve pages for matrixes" "\n"
			ANSI_RESET
		);
		proc_exit(255);
	};

	// u8 const SEED = 0;

	u8 const WEIGHT_PREC = 3;
	u8 const PREC = 3;
	u32 _seed = (u32)rnd_seed(); u32 * seed = &_seed;

	#define IN  3
	#define OUT 3

	// Col X = COL_ON_STACK(IN);  Col XG = COL_ON_STACK(IN);
	// Col Y = COL_ON_STACK(OUT); Col YG = COL_ON_STACK(OUT);
	//
	// Lin l0 = (Lin){
	// 	.x = COL_ON_STACK(IN),      .xg = COL_ON_STACK(IN),
	// 	.y = COL_ON_STACK(OUT),     .yg = COL_ON_STACK(OUT),
	// 	.w = MAT_ON_STACK(OUT, IN), .wg = MAT_ON_STACK(OUT, IN),
	// 	.b = COL_ON_STACK(IN),      .bg = COL_ON_STACK(IN),
	// };
	//
	// Lin l1 = (Lin){
	// 	.x = X, .xg = XG,
	// 	.y = Y, .yg = YG,
	// 	.w = MAT_ON_STACK(OUT, IN), .wg = MAT_ON_STACK(OUT, IN),
	// 	.b = COL_ON_STACK(IN),      .bg = COL_ON_STACK(IN),
	// };

	Mat W = MAT_ON_STACK(OUT, IN); mat_rnd(W, seed);
	mat_fmt(fmt, W, .prec = WEIGHT_PREC, .name = S("W"));

	Col X = COL_ON_STACK(IN); col_rnd(X, seed);
	mat_fmt(fmt, MAT_FROM_COL(X), .prec = PREC, .name = S("X"));

	Col B = COL_ON_STACK(OUT); col_rnd(B, seed);
	mat_fmt(fmt, MAT_FROM_COL(B), .prec = PREC, .name = S("B"));

	Col Y = COL_ON_STACK(OUT);
	mat_fma_col(Y, W, X, B);
	mat_fmt(fmt, MAT_FROM_COL(Y), .prec = PREC, .name = S("Y"));

// ok:

	// Fmt tmp = FMT_ON_STACK(KB(4), FMT_SET_TEXT | FMT_SET_COLOR);
	// FMT(&tmp, FMT_STR(fmt_as_str(fmt), .opt = FMT_S_ESCAPE | FMT_S_QUOTES), FMT_LIT("\n"));
	// fmt_flush_stream(&tmp, out);
	fmt_flush_stream(fmt, out);
	proc_exit(0);

// err:
	fmt_flush_stream(fmt, out);
	proc_exit(1);
};

