
#include "mat/src/ten.h"
#include "std/fmt/core.h"
#include "std/mem/reg.h"
#include "std/rnd/core.h"

f32 f32_abs(f32 x) {
	union { f32 f; u32 u; } u = { .f = x };
	u.u &= 0x7FFFFFFFu;
	return u.f;
};

f32 f32_sign(f32 x) {
	if (x > 0.0f) return +1.0f;
	if (x < 0.0f) return -1.0f;
	return 0.0f;
};

b8 f32_finite(f32 x) {
	union { f32 f; u32 u; } bits = { .f = x };
	return (bits.u & 0x7F800000u) != 0x7F800000u;
};

// b8 f32_nan(f32 x) { return x != x; }

// |================================================================================================|
// |> TENSOR DIMENSIONS                                                                             |

u32 ten_dims_size(TenDims dims) {
	if (dims.count == 0) return 0;
	u32 total = 1;
	for (u32 i = 0; i < dims.count; i += 1) total *= dims.arr[i];
	return total;
};

// |================================================================================================|
// |> TENSOR SLICING                                                                                |

// |================================================================================================|
// |> TENSOR API                                                                                    |

b8 ten_init(Tensor * ten, TenDims dims, RegMan man) {
	if (dims.count == 0) return false;

	u32 size = ten_dims_size(dims); f32 * floats = 0;
	Reg const reg = reg_upd_arr(man, floats, 0, size, REG_DIR_UP);
	if (!reg.is_valid) return false;

	ten->ptr = reg.any; ten->offset = 0;
	ten->dims = dims; u32 i = dims.count;
	i -= 1; ten->strides[i] = 1;

	for (; i != 0; i -= 1) {
		ten->strides[i - 1] = ten->strides[i] * ten->dims.arr[i];
	};

	return true;
};

// |================================================================================================|
// |> TENSOR FORMAT                                                                                 |

b8 ten_fmt_dims_ex(Fmt * fmt, Tensor const * ten, TenFmtOpt opt) {
	if (str_sane(opt.name)) {
		FMT(fmt,
			FMT_CYAN, FMT_STR(opt.name),
			FMT_YELLOW, FMT_LIT(" = "),
		);
	};

	FMT(fmt, FMT_LIT("["));
	for (u32 i = 0; i < ten->dimension; i += 1) {
		if (i != 0) FMT(fmt, FMT_YELLOW, FMT_LIT(","));
		FMT(fmt, FMT_BLUE, FMT_U64(ten->ds[i]));
	};
	FMT(fmt, FMT_YELLOW, FMT_LIT("]"), FMT_CYAN, FMT_LIT("f32"));

	FMT(fmt, FMT_YELLOW, FMT_LIT(" ("));
	for (u32 i = 0; i < ten->dimension; i += 1) {
		if (i != 0) FMT(fmt, FMT_YELLOW, FMT_LIT(":"));
		FMT(fmt, FMT_MAGENTA, FMT_U64(ten->strides[i]));
	};
	FMT(fmt, FMT_YELLOW, FMT_LIT(") "));

	FMT(fmt, FMT_ORANGE, FMT_MEM(ten_dims_size(ten->dims) * sizeof(*ten->ptr)));

	FMT(fmt, FMT_LIT("\n"), FMT_RESET);

	return true;
};

// |================================================================================================|
// |> Tenzor                                                                                        |

void t2_rnd(T2 m, u32 * seed) {
	t1_rnd((T1){.ptr = m.ptr, .len = m.d1 * m.d2}, seed);
};

void t1_rnd(T1 c, u32 * seed) {
	for (u64 i = 0; i < c.len; i += 1) c.ptr[i] = rnd_bi(seed);
};

b8 t2_alloc(T2 * mat, RegMan man, u32 row, u32 col) {
	if (mat->ptr != 0) return false;

	f32 * floats = 0;
	Reg const reg = reg_upd_arr(man, floats, 0, row * col, REG_DIR_UP);

	if (reg.is_valid) {
		mat->ptr = reg.any; mat->d1 = row; mat->d2 = col;
		return true;
	} else {
		mat->ptr = 0; mat->d1 = 0; mat->d2 = 0;
		return false;
	};
};

b8 t1_alloc(T1 * col, RegMan man, u32 len) {
	if (col->ptr != 0) return false;

	f32 * floats = 0;
	Reg const reg = reg_upd_arr(man, floats, 0, len, REG_DIR_UP);

	if (reg.is_valid) {
		col->ptr = reg.any; col->len = len;
		return true;
	} else {
		col->ptr = 0; col->len = 0;
		return false;
	};
};

// typedef enum: u8 {
//
// };

b8 t2_fma_t1(T1 dst, T2 w, T1 x, T1 b) {
	if (w.d1 != dst.len || w.d2 != x.len || w.d1 != b.len) return false;

	for (u64 i = 0; i < dst.len; i += 1) {
		f32 a = 0.0;
		for (u64 j = 0; j < x.len; j += 1) {
			a += w.ptr[i * w.d1 + j] * x.ptr[j];
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
} TenColor;

static TenColor t2_color(f32 x, TenRange range) {
	if (x != x) {
		const u8 nr = 255, ng = 0,   nb = 255;
		return (TenColor){ .r = nr, .g = ng, .b = nb };
	};

	f32 zero_check = x * 0.0f;
	if (zero_check != 0.0f) { 
		if (x > 0.0f) {
			const u8 pr = 255, pg = 255, pb = 255;
			return (TenColor){ .r = pr, .g = pg, .b = pb };
		} else {
			const u8 mr = 255, mg = 0, mb = 255;
			return (TenColor){ .r = mr, .g = mg, .b = mb };
		};
	};

	const u8 br = 80,  bg = 140, bb = 255;
	const u8 gr = 180, gg = 180, gb = 180;
	const u8 rr = 255, rg = 80,  rb = 80;

	f32 mid = (range.min + range.max) * 0.5f;
	f32 t;

	if (x < mid) {
		t = (x - range.min) / (mid - range.min);
		t = t < 0 ? 0 : t > 1 ? 1 : t;

		return (TenColor){
			.r = lerp_u8(br, gr, t),
			.g = lerp_u8(bg, gg, t),
			.b = lerp_u8(bb, gb, t),
		};
	};

	t = (x - mid) / (range.max - mid);
	t = t < 0 ? 0 : t > 1 ? 1 : t;

	return (TenColor){
		.r = lerp_u8(gr, rr, t),
		.g = lerp_u8(gg, rg, t),
		.b = lerp_u8(gb, rb, t),
	};
};

b8 t2_fmt_color(Fmt * fmt, f32 x, TenStyle style) {
	TenColor c = t2_color(x, style.range);
	// return FMT(fmt, FMT_LIT("\x1b[38:2::104:104:104m"));
	return FMT(fmt,
		FMT_LIT("\x1b[38;2;", .flow = FMT_FLOW_COLOR),
		FMT_U64(c.r, .flow = FMT_FLOW_COLOR),
		// FMT_U64(104, .flow = FMT_FLOW_COLOR),
		FMT_LIT(";", .flow = FMT_FLOW_COLOR),
		FMT_U64(c.g, .flow = FMT_FLOW_COLOR),
		// FMT_U64(104, .flow = FMT_FLOW_COLOR),
		FMT_LIT(";", .flow = FMT_FLOW_COLOR),
		FMT_U64(c.b, .flow = FMT_FLOW_COLOR),
		// FMT_U64(104, .flow = FMT_FLOW_COLOR),
		FMT_LIT("m", .flow = FMT_FLOW_COLOR),
	);
};

b8 t2_fmt_item(Fmt * fmt, f32 x, TenStyle style) {
	t2_fmt_color(fmt, x, style);

	if (true /* style.prec != 0 */) return FMT(fmt,
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

// b8 t2_fmt_prec_zero(Fmt * fmt, T2 t2, TenFmtOpt opt) {
// 	return false;
// };

b8 t2_fmt_ex(Fmt * fmt, T2 t2, TenFmtOpt opt) {
	if (opt.prec == 0) opt.prec = 3;
	if (opt.d1 == 0) opt.d1 = (t2.d1 > 8) ? 3 : (u32)t2.d1;
	if (opt.d2 == 0) opt.d2 = (t2.d2 > 8) ? 3 : (u32)t2.d2;
	if (opt.max == opt.min) { opt.min = -1.0; opt.max = +1.0; };

	// if (opt.line == 0 && t2.d1 == 1) {
	// 	opt.line = opt.d1;
	// } else {
	// 	opt.line = MIN(opt.line, opt.d1);
	// };
	opt.line = MIN(opt.line, opt.d1);

	u32 offset = 0;
	if (str_sane(opt.name)) {
		FMT(fmt,
			FMT_CYAN, FMT_STR(opt.name),
			FMT_YELLOW, FMT_LIT(" = "),
			FMT_ORANGE, FMT_T2(t2),
			FMT_LIT(" "),
		);

		offset = (u32)fmt->last.len;
		if (fmt->flow & FMT_SET_COLOR) {
			offset -= S(ANSI_CYAN).len + S(ANSI_YELLOW).len + S("\x1B[38;5;208m").len;
		};
	};

	if (t2.d1 == 0 || t2.d2 == 0) {
		FMT(fmt, FMT_YELLOW, FMT_LIT("[]\n"));
		return true;
	};

	u64 const d1_max = MIN(t2.d1, opt.d1);
	u64 const d2_max = MIN(t2.d2, opt.d2) - 1;
	b8  const newline = opt.line < d2_max;

	for (u64 i = 0; i < d1_max; i += 1) {
		Str const beg = (i == 0) ? S("[[") : S(" [");
		Str const end = (i == t2.d1 - 1)
			? S("]]\n")
			: (newline) ? S("],\n") : S("],");

		if (newline) {
			u32 repeat = (i == 0) ? 0 : offset;
			FMT(fmt, FMT_REPEAT(' ', repeat));
		};

		FMT(fmt, FMT_YELLOW, FMT_STR(beg));

		for (u64 j = 0; j < d2_max; j += 1) {
			f32 x = t2.ptr[i * t2.d2 + j];
			t2_fmt_item(fmt, x, opt.style);
			FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
		};

		if (d2_max < t2.d2 - 1) {
			FMT(fmt, FMT_YELLOW, FMT_LIT("..."));
		} else {
			f32 x = t2.ptr[i * t2.d2 + d2_max];
			t2_fmt_item(fmt, x, opt.style);
		};

		FMT(fmt, FMT_YELLOW);
		if (i == d1_max - 1 && d1_max < t2.d1) {
			FMT(fmt, FMT_LIT("], ...]\n"));
		} else {
			FMT(fmt, FMT_STR(end));
		};
	};

	return true;
};

b8 lin_init(
	Lin * lin,
	B1 xb, u32 out,
	TenMeta * meta
) {
	u32 const in = xb.v.len;

	lin->xb = xb;
	if (!t2_alloc(&lin->w, meta->man, out, in)) return false;
	if (!t1_alloc(&lin->b, meta->man, out))     return false;
	if (!t1_alloc(&lin->y, meta->man, out))     return false;
	lin->meta = meta;

	if (meta->learn == TEN_LEARN_YES) {
		if (!t2_alloc(&lin->wg, meta->man, out, in)) return false;
		if (!t1_alloc(&lin->bg, meta->man, out))     return false;
		if (!t1_alloc(&lin->yg, meta->man, out))     return false;
	} else {
		lin->wg = (T2){0};
		lin->bg = (T1){0};
		lin->yg = (T1){0};
	};

	return true;
};

b8 lin_fmt(Fmt * fmt, Lin const * lin) {
	b8 ok = true;

	// ok &= t1_fmt(fmt, lin->x, .prec = 3, .name = S("X"));
	ok &= t2_fmt(fmt, lin->w, .prec = 3, .name = S("W"));
	ok &= t1_fmt(fmt, lin->b, .prec = 1, .name = S("B"));
	// ok &= t1_fmt(fmt, lin->y, .prec = 1, .name = S("Y"));

	if (lin->wg.ptr != 0) ok &= t2_fmt(fmt, lin->wg, .prec = 6, .name = S("G(W)"));
	if (lin->bg.ptr != 0) ok &= t1_fmt(fmt, lin->bg, .prec = 6, .name = S("G(B)"));

	return ok;
};

b8 lin_rnd(Lin * lin, u32 * seed) {
	f64 const limit = 1.0 / (f64)lin->w.d2;

	for (u64 i = 0; i < lin->w.d1 * lin->w.d2; i += 1) {
		lin->w.ptr[i] = rnd_bi(seed) * (f32)limit;
	};

	for (u64 i = 0; i < lin->b.len; i += 1) lin->b.ptr[i] = 0.0f;
	for (u32 i = 0; i < lin->wg.d1 * lin->wg.d2; i += 1) lin->wg.ptr[i] = 0.0f;
	for (u32 i = 0; i < lin->bg.len; i += 1) lin->bg.ptr[i] = 0.0f;
	return true;
};

b8 lin_eval(Lin * lin) {
	return t2_fma_t1(lin->y, lin->w, lin->x, lin->b);
};

b8 lin_back(Lin * lin) {
	if (lin->wg.ptr == 0 || lin->bg.ptr == 0 || lin->yg.ptr == 0) return true;

	for (u32 i = 0; i < lin->wg.d1; i += 1) {
		lin->bg.ptr[i] += lin->yg.ptr[i];
		for (u32 j = 0; j < lin->wg.d2; j += 1) {
			lin->wg.ptr[i * lin->wg.d2 + j] += lin->x.ptr[j] * lin->yg.ptr[i];
			if (lin->xg.ptr != 0) lin->xg.ptr[j] += lin->w.ptr[i * lin->wg.d2 + j] * lin->yg.ptr[i];
		};
	};

	return true;
};

b8 lin_learn(Lin * lin) {
	if (lin->wg.ptr == 0 || lin->bg.ptr == 0) return true;

	f32 const step = lin->meta->step;

	for (u32 i = 0; i < lin->bg.len; i += 1) {
		lin->b.ptr[i] -= step * f32_sign(lin->bg.ptr[i]);
		lin->bg.ptr[i] = 0.0f;
	};

	for (u32 i = 0; i < lin->wg.d1 * lin->wg.d2; i += 1) {
		lin->w.ptr[i] -= step * f32_sign(lin->wg.ptr[i]);
		lin->wg.ptr[i] = 0.0f;
	};

	return true;
};

b8 relu_init(
	Relu * relu,
	B1 xb, f32 a,
	TenMeta * meta
) {
	relu->xb = xb;
	relu->a  = a;

	u32 const out = xb.v.len;

	if (!t1_alloc(&relu->y, meta->man, out)) return false;
	relu->meta = meta;

	if (meta->learn == TEN_LEARN_YES) {
		if (!t1_alloc(&relu->yg, meta->man, out)) return false;
	} else {
		relu->yg = (T1){0};
	};

	return true;
};

b8 relu_eval(Relu * relu) {
	for (u32 i = 0; i < relu->x.len; i += 1) {
		f32 const x = relu->x.ptr[i];
		relu->y.ptr[i] = (x > 0.0f) ? x : x * relu->a;
	};
	return true;
};

b8 relu_back(Relu * relu) {
	for (u32 i = 0; i < relu->x.len; i += 1) {
		f32 const x = relu->x.ptr[i];
		f32 const s = (x > 0.0f) ? 1.0 : relu->a;
		relu->xg.ptr[i] += s * relu->yg.ptr[i];
	};
	return true;
};

b8 relu_learn(Relu * relu) { UNUSED(relu);
	// nothing to do
	return true;
};

b8 add_eval(Add * add);
b8 add_learn(Add * add);
b8 add_step(Add * add);

Insts insts_from_slice(Inst * ptr, u32 len);
b8    insts_push(Insts * insts, Inst item);
void  insts_reset(Insts * insts);

