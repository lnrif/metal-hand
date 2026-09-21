#include "mat/src/mat.h"
#include "std/fmt/core.h"
#include "std/rnd/core.h"

// |================================================================================================|
// |> Matrix                                                                                        |

void mat_rnd(Mat m, u32 * seed) {
	col_rnd((Col){.ptr = m.ptr, .len = m.row * m.col}, seed);
};

void col_rnd(Col c, u32 * seed) {
	for (u64 i = 0; i < c.len; i += 1) c.ptr[i] = rnd_bi(seed);
};

b8 mat_fma_col(Col dst, Mat w, Col x, Col b) {
	if (w.row != dst.len || w.col != x.len || w.row != b.len) return false;

	for (u64 i = 0; i < dst.len; i += 1) {
		f32 a = 0.0;
		for (u64 j = 0; j < x.len; j += 1) {
			a += w.ptr[i * w.col + j] * x.ptr[j];
		};
		dst.ptr[i] = a + b.ptr[i];
	};

	return true;
};

static u8 lerp_u8(u8 a, u8 b, f32 t) {
	return (u8)((f32)a + ((f32)b - (f32)a) * t);
};

typedef struct {
	u8 r, g, b;
} MatColor;

static MatColor mat_color(f32 x, MatRange range) {
	const u8 br = 80,  bg = 140, bb = 255;
	const u8 gr = 180, gg = 180, gb = 180;
	const u8 rr = 255, rg = 80,  rb = 80;

	f32 mid = (range.min + range.max) * 0.5f;
	f32 t;

	if (x < mid) {
		t = (x - range.min) / (mid - range.min);
		t = t < 0 ? 0 : t > 1 ? 1 : t;

		return (MatColor){
			.r = lerp_u8(br, gr, t),
			.g = lerp_u8(bg, gg, t),
			.b = lerp_u8(bb, gb, t),
		};
	};

	t = (x - mid) / (range.max - mid);
	t = t < 0 ? 0 : t > 1 ? 1 : t;

	return (MatColor){
		.r = lerp_u8(gr, rr, t),
		.g = lerp_u8(gg, rg, t),
		.b = lerp_u8(gb, rb, t),
	};
};

b8 mat_fmt_color(Fmt * fmt, f32 x, MatStyle style) {
	MatColor c = mat_color(x, style.range);
	return FMT(fmt,
		FMT_LIT("\x1B[38;2;", .flow = FMT_FLOW_COLOR),
		FMT_U64(c.r, .flow = FMT_FLOW_COLOR),
		FMT_LIT(";", .flow = FMT_FLOW_COLOR),
		FMT_U64(c.g, .flow = FMT_FLOW_COLOR),
		FMT_LIT(";", .flow = FMT_FLOW_COLOR),
		FMT_U64(c.b, .flow = FMT_FLOW_COLOR),
		FMT_LIT("m", .flow = FMT_FLOW_COLOR),
	);
};

b8 mat_fmt_item(Fmt * fmt, f32 x, MatStyle style) {
	mat_fmt_color(fmt, x, style);

	if (style.prec != 0) return FMT(fmt,
		FMT_F64(x,
			.prec = style.prec,
			.opt = FMT_N_SIGN,
		),
	);

	Str s;
	if (x < 0) {
		s = S("-");
	} else if (x > 0) {
		s = S("+");
	} else {
		s = S("=");
	};

	return fmt_lit_ex(fmt, s);
};

b8 mat_fmt_ex(Fmt * fmt, Mat mat, MatStyle style) {
	if (style.range.max == style.range.min) {
		style.range.min = -1.0;
		style.range.max = +1.0;
	};

	u32 offset = 0;
	if (str_sane(style.name)) {
		offset = (u32)(style.name.len + S(" = ").len);
		FMT(fmt,
			FMT_CYAN, FMT_STR(style.name),
			FMT_YELLOW, FMT_LIT(" = "),
		);
	};

	for (u64 i = 0; i < mat.row; i += 1) {
		Str beg = (i == 0)           ? S("[[")   : S(" [");
		Str end = (i == mat.row - 1) ? S("]]\n") : S("],\n");
		u32 repeat = (i == 0) ? 0 : offset;
		FMT(fmt, FMT_REPEAT(' ', repeat), FMT_YELLOW, FMT_STR(beg));

		Str delim = style.prec == 0 ? S("") : S(", ");
		for (u64 j = 0; j < mat.col - 1; j += 1) {
			f32 x = mat.ptr[i * mat.col + j];
			mat_fmt_item(fmt, x, style);
			FMT(fmt, FMT_YELLOW, FMT_STR(delim));
		};

		f32 x = mat.ptr[i * mat.col + mat.col - 1];
		mat_fmt_item(fmt, x, style);

		FMT(fmt, FMT_YELLOW, FMT_STR(end));
	};

	return true;
};

b8 lin_eval(Lin * lin);
b8 lin_learn(Lin * lin);
b8 lin_step(Lin * lin);

b8 relu_eval(Relu * relu);
b8 relu_learn(Relu * relu);
b8 relu_step(Relu * relu);

b8 add_eval(Add * add);
b8 add_learn(Add * add);
b8 add_step(Add * add);

Insts insts_from_slice(Inst * ptr, u32 len);
b8    insts_push(Insts * insts, Inst item);
void  insts_reset(Insts * insts);

