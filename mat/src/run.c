#include "std/core.h"
#include "std/mem/reg.h"
#include "std/proc/exit.h"
#include "std/run/run.h"

#include "std/mem/page.h"
#include "std/fmt/core.h"

#include "mat/src/ten.h"

// #define MAT_ALLOC_OR_FAIL(mat, out, in, exit...)

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

	u8 error_code = 0;

	Pages mat_pages = pages_reserve(MB(16));
	if (!mat_pages.is_valid) {
		FMT(fmt,
			FMT_BOLD, FMT_RED, FMT_LIT("[fatal] "),
			FMT_WHITE, FMT_LIT("failed to reserve pages for matrixes" "\n"),
			FMT_RESET,
		);
		error_code = 255;
		goto exit;
	};

	RegMan man = pages_pinned(&mat_pages);
	// u32 _seed = (u32)rnd_seed(); u32 * seed = &_seed;

	Tensor x0 = {0};
	if (!ten_init(&x0, TEN_DIMS(3), man)) { error_code = 69; goto exit; };

	Tensor w0 = {0};
	if (!ten_init(&w0, TEN_DIMS(4, 3), man)) { error_code = 69; goto exit; };

	Tensor b0 = {0};
	if (!ten_init(&b0, TEN_DIMS(4), man)) { error_code = 69; goto exit; };

	ten_fmt_dims(fmt, &x0, .name = S("X"));
	ten_fmt(fmt, &x0, .prec = 3, .params = 32, .line = 0);

	ten_fmt_dims(fmt, &w0, .name = S("W"));
	ten_fmt(fmt, &w0, .prec = 3, .params = 32, .line = 0);

	// change prec to 5
	ten_fmt_dims(fmt, &b0, .name = S("B"));
	ten_fmt(fmt, &b0, .prec = 3, .params = 32, .line = 0);

	fmt_flush_stream(fmt, out);

	// for (;;) {
	// 	FMT(fmt, FMT_MAGENTA, FMT_LIT("\n"), FMT_REPEAT('-', 64), FMT_LIT("\n"));
	// 	ten_fmt_dims(fmt, &w0, .name = S("W0"));
	// 	ten_fmt(fmt, &w0, .prec = 1, .params = 64);
	// 	fmt_flush_stream(fmt, out);
	//
	// 	if (w0.dimension == 0) break;
	// 	Tensor t = {0}; TenSlice s = TEN_SLICE(DIM_INDEX(0));
	// 	ten_slice(&w0, &t, &s);
	// 	w0 = t;
	// };

	// Tensor b0 = {0};
	// if (!ten_init(&b0, TEN_DIMS(1, 1), man)) { error_code = 69; goto exit; };
	// ten_fmt_dims(fmt, &b0, .name = S("B0"));


	// Tensor c; TenSlice cs = TEN_SLICE(DIM_SLICE(1, 2));
	// if (!ten_slice(&a, &c, &cs)) { error_code = 69; goto exit; };
	// ten_fmt_dims(fmt, &c, .name = S("C"));

	// TenMeta meta = (TenMeta){.man = man, .learn = TEN_LEARN_YES, .step = 0.0001f};
	// B1 y; Lin l0; Relu r1; Lin l2;
	//
	// if (!t1_alloc(&y.v, meta.man, IN)) {
	// 	error_code = 69;
	// 	goto exit;
	// };
	//
	// if (!lin_init(&l0, y, 2, &meta)) {
	// 	error_code = 69;
	// 	goto exit;
	// };
	//
	// if (!relu_init(&r1, l0.yb, 0.2f, &meta)) {
	// 	error_code = 69;
	// 	goto exit;
	// };
	//
	// if (!lin_init(&l2, r1.yb, OUT, &meta)) {
	// 	error_code = 69;
	// 	goto exit;
	// };
	//
	// T1 xs[10]; T1 ys[LEN(xs)];
	// for (u32 i = 0; i < LEN(xs); i += 1) {
	// 	if (!t1_alloc(&xs[i], meta.man, IN)) {
	// 		error_code = 69;
	// 		goto exit;
	// 	};
	//
	// 	if (!t1_alloc(&ys[i], meta.man, OUT)) {
	// 		error_code = 69;
	// 		goto exit;
	// 	};
	//
	// 	f32 const a = (f32)(i + 1);
	//
	// 	xs[i].ptr[0] = a / (f32)LEN(xs);
	// 	ys[i].ptr[0] = (a + 1.0f) / (f32)(LEN(xs) + 1);
	// };
	//
	// lin_rnd(&l0, seed); lin_rnd(&l2, seed);
	//
	// FMT(fmt, FMT_REPEAT('=', 64), FMT_LIT("\n"));
	// lin_fmt(fmt, &l0);
	// FMT(fmt, FMT_REPEAT('.', 64), FMT_LIT("\n"));
	// lin_fmt(fmt, &l2);
	// FMT(fmt, FMT_LIT("\n"));
	//
	// f32 cost = 10000.0f;
	// for (u32 i = 0; i < 10000000 && f32_abs(cost) > 0.0000001; i += 1) {
	// 	cost = 0.0f;
	// 	for (u32 j = 0; j < LEN(xs); j += 1) {
	// 		l0.x = xs[j];
	// 		// t1_fmt(fmt, l0.x);
	//
	// 		lin_eval(&l0);
	// 		relu_eval(&r1);
	// 		lin_eval(&l2);
	//
	// 		for (u32 k = 0; k < l2.y.len; k += 1) {
	// 			f32 const g = l2.y.ptr[k] - ys[j].ptr[k];
	// 			l2.yg.ptr[k] = g;
	// 			cost += f32_abs(g);
	// 		};
	//
	// 		// t1_fmt(fmt, xs[j], .name = S("X"));
	// 		// t1_fmt(fmt, ys[j], .name = S("Y"));
	// 		// t1_fmt(fmt, l2.y,  .name = S("O"));
	//
	// 		lin_back(&l2);
	// 		relu_back(&r1);
	// 		lin_back(&l0);
	//
	// 		// lin_learn(&l0); lin_learn(&l2);
	// 	};
	//
	// 	if (i % 500000 == 0) {
	// 		FMT(fmt,
	// 			FMT_BLUE,
	// 			FMT_LIT("["), FMT_U64(i, .digits = 4), FMT_LIT("] "),
	// 			FMT_YELLOW,
	// 			FMT_LIT("cost: "),
	// 			FMT_ORANGE,
	// 			FMT_F64(cost, .prec = 3),
	// 			FMT_LIT("\n"),
	// 			FMT_RESET,
	// 		);
	// 		FMT(fmt, FMT_REPEAT('=', 64), FMT_LIT("\n"));
	// 		lin_fmt(fmt, &l0);
	// 		FMT(fmt, FMT_REPEAT('.', 64), FMT_LIT("\n"));
	// 		lin_fmt(fmt, &l2);
	// 		FMT(fmt, FMT_LIT("\n"));
	// 		fmt_flush_stream(fmt, out);
	// 	};
	//
	// 	lin_learn(&l0); lin_learn(&l2);
	// };
	//
	// FMT(fmt, FMT_REPEAT('=', 64), FMT_LIT("\n"));
	// lin_fmt(fmt, &l0);
	// FMT(fmt, FMT_REPEAT('.', 64), FMT_LIT("\n"));
	// lin_fmt(fmt, &l2);
	//
	// FMT(fmt,
	// 	FMT_YELLOW,
	// 	FMT_LIT("cost: "),
	// 	FMT_ORANGE,
	// 	FMT_F64(cost, .prec = 3),
	// 	FMT_LIT("\n"),
	// 	FMT_RESET,
	// );
	//
	// FMT(fmt, FMT_REPEAT('=', 64), FMT_LIT("\n"));
	// lin_fmt(fmt, &ll);

exit:
	fmt_flush_stream(fmt, out);
	proc_exit(error_code);
};

