#include "std/core.h"
#include "std/proc/exit.h"
#include "std/run/run.h"

#include "std/mem/page.h"
#include "std/fmt/core.h"

// #include "std/flow/core.h"

#include "mat/src/mat.h"

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

	u8 const SEED = 0;

	u8 const WEIGHT_PREC = 1;
	u8 const PREC = 1;
	u32 _seed = (u32)(u64)&SEED; u32 * seed = &_seed;

	#define IN  2
	#define OUT 2

	Col X = COL_ON_STACK(IN);  Col XG = COL_ON_STACK(IN);
	Col Y = COL_ON_STACK(OUT); Col YG = COL_ON_STACK(OUT);

	Lin l0 = (Lin){
		.x = X, .xg = XG,
		.y = Y, .yg = YG,
		.w = MAT_ON_STACK(OUT, IN), .wg = MAT_ON_STACK(OUT, IN),
		.b = COL_ON_STACK(IN),      .bg = COL_ON_STACK(IN),
	};

	Lin l1 = (Lin){
		.x = X, .xg = XG,
		.y = Y, .yg = YG,
		.w = MAT_ON_STACK(OUT, IN), .wg = MAT_ON_STACK(OUT, IN),
		.b = COL_ON_STACK(IN),      .bg = COL_ON_STACK(IN),
	};


	Mat W = MAT_ON_STACK(OUT, IN); mat_rnd(W, seed);
	mat_fmt(fmt, W, .prec = WEIGHT_PREC, .name = S("W"));

	// Col X = COL_ON_STACK(IN); col_rnd(X, seed);
	mat_fmt(fmt, MAT_FROM_COL(X), .prec = PREC, .name = S("X"));

	Col B = COL_ON_STACK(OUT); col_rnd(B, seed);
	mat_fmt(fmt, MAT_FROM_COL(B), .prec = PREC, .name = S("B"));

	Col Y = COL_ON_STACK(OUT);
	mat_fma_col(Y, W, X, B);
	mat_fmt(fmt, MAT_FROM_COL(Y), .prec = PREC, .name = S("Y"));

	// Inst _insts_buf[16] = {0};
	// Insts insts = insts_from_slice(_insts_buf, LEN(_insts_buf));
	//
	// insts_push(&insts, (Inst){
	// 	.as.lin = (Lin){
	//
	// 	},
	// 	.tag = INST_TAG_LINEAR,
	// });

	// Mat a = MAT_ON_STACK(2, 2);
	// a.ptr[0] = 1.0; a.ptr[1] = 2.0;
	// a.ptr[2] = 3.0; a.ptr[3] = 4.0;
	//
	// Col b = COL_ON_STACK(2);
	// b.ptr[0] = 2.0; b.ptr[1] = -2.0;
	//
	// // for (u64 i = 0; i < a.row * a.col; i += 1)
	// // 	a.ptr[i] = ((f32)i + 1 - 10) / (f32)(a.row * a.col);
	//
	// mat_fmt(fmt, a,               .prec = 0, .name = S("a"));
	// mat_fmt(fmt, MAT_FROM_COL(b), .prec = 0, .name = S("b"));
	//
	// Col c = COL_ON_STACK(2);
	// c.ptr[0] = -1.0; c.ptr[1] = +1.0;
	// mat_fmt(fmt, MAT_FROM_COL(c), .prec = 0, .name = S("c"));
	//
	// Col d = COL_ON_STACK(2);
	// mat_fma_col(d, a, b, c);
	// mat_fmt(fmt, MAT_FROM_COL(d), .prec = 0, .name = S("d"));

	// Mat W = MAT_ON_STACK(1, 1);
	// Col B = COL_ON_STACK(1);
	// f32 S = 0.1f;


	// Mat O = MAT_ON_STACK(4, 1);
	// O.ptr = (f32[]){1.0, 3.0, 7.0, 5.0};
	//
	// ASSERT(I.row == O.row, "input and output mismatch dimensions");
	//
	// for (u64 i = 0; i < 100; i += 1) {
	// 	Col gb = COL_ON_STACK(1);
	// 	Mat gw = MAT_ON_STACK(1, 1);
	//
	// 	for (u64 j = 0; j < I.row; j += 1) {
	// 		for (u64 k = 0; k < O.row; k += 1) {
	// 			Col x = (Col){.ptr = &I.ptr[I.col * j], .len = I.col};
	// 			Col e = (Col){.ptr = &O.ptr[O.col * j], .len = O.col};
	// 			Col y = COL_ON_STACK(1);
	//
	// 			// y = wx + b;
	// 			mat_fma_col(y, W, x, B);
	//
	// 			Col g = COL_ON_STACK(1);
	// 			// g = S * (y - e);
	// 			col_sub(g, y, e);
	// 			col_scale(g, g, S);
	// 			// gb += g;
	// 			col_add(gb, gb, g);
	// 			// gw += g * x;
	// 			col_mul(g, g, x);
	// 			col_add(gw, gw, g);
	// 		};
	// 	};
	//
	// 	// b -= g;
	// 	col_sub(B, B, gb);
	//
	// 	for (u64 j = 0; j < I.col; j += 1) {
	// 		Col x = (Col){.ptr = &I.ptr[I.col * j], .len = I.col};
	// 		// Col x = (Col){.ptr = &O.ptr[O.col * j], .len = O.col};
	// 	};
	//
	// 	// w -= g * x;
	// 	col_mul(g, g, x);
	// 	col_sub(w, w, g);
	// };
	//
	// // f32 w = 0.0;
	// // f32 b = 0.0;
	// //
	// // f32 A[2] = {1.0, 4.0};
	// // f32 B[2] = {2.0, 8.0};
	// // f32 S = 0.01f;
	// // for (u64 i = 0; i < 100; i += 1) {
	// // 	for (u64 j = 0; j < 2; j += 1) {
	// // 		f32 y = w * A[j] + b;
	// // 		f32 g = (y - B[j]) * S;
	// // 		w -= g * A[j]; b -= g;
	// // 	};
	// // };
	//
	// mat_fmt(fmt, w, .prec = 3, .name = S("w"));
	// mat_fmt(fmt, MAT_FROM_COL(b), .prec = 3, .name = S("b"));

	// FMT(fmt,
		// FMT_CYAN, FMT_LIT("g = "), FMT_MAGENTA, FMT_F64(g, .prec = 3), FMT_LIT("\n"),
		// FMT_CYAN, FMT_LIT("w = "), FMT_MAGENTA, FMT_F64(w.ptr, .prec = 2), FMT_LIT("\n"),
		// FMT_CYAN, FMT_LIT("b = "), FMT_MAGENTA, FMT_F64(b, .prec = 2), FMT_LIT("\n"),
		// FMT_LIT("\n"),
	// );

	// u8 const digits = 2;
	// FMT(fmt,
	// 	FMT_BOLD,
	// 	FMT_YELLOW, FMT_LIT("[["),
	// 	FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.457, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.568, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	// );
	//
	// FMT(fmt,
	// 	FMT_BOLD,
	// 	FMT_YELLOW, FMT_LIT(" ["),
	// 	FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.568, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	// );
	//
	// FMT(fmt,
	// 	FMT_BOLD,
	// 	FMT_YELLOW, FMT_LIT(" ["),
	// 	FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.568, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	// );
	//
	// FMT(fmt,
	// 	FMT_BOLD,
	// 	FMT_YELLOW, FMT_LIT(" ["),
	// 	FMT_RED, FMT_F64(+0.124, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.568, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT("],"), FMT_LIT("\n"),
	// );
	//
	// FMT(fmt,
	// 	FMT_BOLD,
	// 	FMT_YELLOW, FMT_LIT(" ["),
	// 	FMT_BLUE, FMT_F64(-0.124, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_BLUE, FMT_F64(-0.235, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.346, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.457, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT(", "),
	// 	FMT_RED, FMT_F64(+0.568, .digits = digits, .opt = FMT_N_SIGN),
	// 	FMT_YELLOW, FMT_LIT("]]"), FMT_LIT("\n"),
	// );

// ok:
	fmt_flush_stream(fmt, out);
	proc_exit(0);

// err:
	fmt_flush_stream(fmt, out);
	proc_exit(1);
};

