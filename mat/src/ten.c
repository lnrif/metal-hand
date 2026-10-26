#include "mat/src/ten.h"
#include "std/core.h"
#include "std/fmt/core.h"
#include "std/mem/reg.h"
#include "std/rnd/core.h"
#include <string.h>

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
	u32 total = 1;
	for (u32 i = 0; i < dims.count; i += 1) total *= dims.arr[i];
	return total;
};

// |================================================================================================|
// |> TENSOR SLICING                                                                                |

void dim_slice_restore(DimSlice * slice) {
	if (slice->step == 0) slice->step = 1;
};

// 2x3x4[0][:][:] -> 3x4

b8 ten_slice(Tensor const * in, Tensor * out, TenSlice * slice) {
	if (in->dimension < slice->count) return false;

	for (u32 i = slice->count; i < in->dimension; i += 1) slice->ds[i] = DIM_FULL();
	slice->count = in->dimension;

	for (u32 i = 0; i < slice->count; i += 1) {
		DimSlice * s = &slice->ds[i]; dim_slice_restore(s);

		if (s->step == DIM_END) {
			if (s->beg >= in->ds[i]) {
				slice->illegal = i; slice->kind = TEN_SLICE_INDEX_OUT_OF_BOUNDS;
				return false;
			};
		} else {
			if (s->end == DIM_END) s->end = in->ds[i];

			if (s->beg >= in->ds[i]) {
				slice->illegal = i; slice->kind = TEN_SLICE_SLICE_OUT_OF_BOUNDS;
				return false;
			};

			if (s->end > in->ds[i]) {
				slice->illegal = i; slice->kind = TEN_SLICE_SLICE_OUT_OF_BOUNDS;
				return false;
			};
		};
	};

	out->ptr = in->ptr; out->offset = in->offset;

	// dense tensor:
	// w[i][j][k] = w + i * d3 + j * d2 + k * 1;

	// tensor + strides:
	// w[i] = w + i[0] * s[0] + i[1] * s[1] + i[2] * s[2]

	for (u32 i = 0; i < in->dimension; i += 1) {
		DimSlice const s = slice->ds[i];
		if (s.step == DIM_END) {
			out->offset += in->strides[i] * s.beg;
		} else {
			out->offset += in->strides[i] * s.beg;
			out->ds[out->dimension] = s.end - s.beg;
			out->strides[out->dimension] = in->strides[i];
			out->dimension += 1;
		};
	};

	return true;
};

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

void ten_fmt_opt_restore(TenFmtOpt * opt, u32 d0, u32 d1) {
	if (opt->prec == 0) opt->prec = 3;
	if (opt->d0 == 0) opt->d0 = (d0 > 8) ? 2 : (u32)d0;
	if (opt->d1 == 0) opt->d1 = (d1 > 8) ? 2 : (u32)d1;
	if (opt->max == opt->min) { opt->min = -1.0; opt->max = +1.0; };

	// if (opt->line == 0 && d1 == 1) {
	// 	opt->line = opt->d1;
	// } else {
	// 	opt->line = MIN(opt->line, opt->d1);
	// };
	opt->line = MIN(opt->line, opt->d0);
};

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

static u8 lerp_u8(u8 a, u8 b, f32 t) {
	return (u8)((f32)a + ((f32)b - (f32)a) * t);
};

typedef struct {
	u8 r, g, b;
} TenColor;

static TenColor item_color(f32 x, TenRange range) {
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

b8 item_fmt_color(Fmt * fmt, f32 x, TenStyle style) {
	TenColor c = item_color(x, style.range);
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

b8 item_fmt_item(Fmt * fmt, f32 x, TenStyle style) {
	item_fmt_color(fmt, x, style);

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


typedef struct {
	Fmt * fmt;
	TenDims cur;
} TenFmtState;

typedef struct {
	f32 * ptr;
	u32 d0; u32 d1;
	u32 s0; u32 s1;
} Matrix;

#define DEPTH_PADDING 1

b8 vector_fmt_raw(Fmt * fmt, Tensor const * ten, TenFmtOpt const * opt) {
	u64 const total = ten->ds[0];
	u64 const edge = 3;
	b8 const truncated = total > edge * 2;
	u64 const head = truncated ? edge : total;

	FMT(fmt, FMT_YELLOW, FMT_LIT("["));

	for (u64 i = 0; i < head; i += 1) {
		f32 x = ten->ptr[ten->offset + i * ten->strides[0]];
		item_fmt_item(fmt, x, opt->style);
		if (i + 1 < head || truncated) {
			FMT(fmt, FMT_LIT(", "));
		}
	}

	if (truncated) {
		FMT(fmt, FMT_YELLOW, FMT_LIT("... , "));
		for (u64 i = total - edge; i < total; i += 1) {
			f32 x = ten->ptr[ten->offset + i * ten->strides[0]];
			item_fmt_item(fmt, x, opt->style);
			if (i + 1 < total) {
				FMT(fmt, FMT_LIT(", "));
			};
		};
	};

	FMT(fmt, FMT_YELLOW, FMT_LIT("]"));
	return true;
};

b8 matrix_fmt_raw(Fmt * fmt, Matrix const * mat, u8 depth, TenFmtOpt const * opt);

b8 matrix_fmt_raw(Fmt * fmt, Matrix const * mat, u8 depth, TenFmtOpt const * opt) {
	u32 const offset = depth * DEPTH_PADDING;
	if (mat->d0 == 0 || mat->d1 == 0) {
		FMT(fmt, FMT_YELLOW, FMT_LIT("[]"));
		return true;
	};

	u64 const edge_rows = opt->d0;
	b8 const truncated_rows = (mat->d0 > edge_rows * 2);
	u64 const head_rows = truncated_rows ? edge_rows : mat->d0;
	u64 const tail_rows = truncated_rows ? edge_rows : 0;

	u64 const edge_cols = opt->d1;
	b8 const truncated_cols = (mat->d1 > edge_cols * 2);
	u64 const head_cols = truncated_cols ? edge_cols : mat->d1;
	u64 const tail_cols = truncated_cols ? edge_cols : 0;

	for (u64 i = 0; i < head_rows; i += 1) {
		if (i > 0) FMT(fmt, FMT_REPEAT(' ', offset));
		Str const beg = (i == 0) ? S("[[") : S(" [");
		FMT(fmt, FMT_YELLOW, FMT_STR(beg));

		for (u64 j = 0; j < head_cols; j += 1) {
			f32 x = mat->ptr[i * mat->s0 + j * mat->s1];
			item_fmt_item(fmt, x, opt->style);
			if (j + 1 < head_cols || truncated_cols || tail_cols > 0) FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
		};

		if (truncated_cols) FMT(fmt, FMT_YELLOW, FMT_LIT("..., "));

		for (u64 j = mat->d1 - tail_cols; j < mat->d1; j += 1) {
			f32 x = mat->ptr[i * mat->s0 + j * mat->s1];
			item_fmt_item(fmt, x, opt->style);
			if (j + 1 < mat->d1) FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
		};

		if (i + 1 < head_rows || truncated_rows) FMT(fmt, FMT_YELLOW, FMT_LIT("],\n"));
		else FMT(fmt, FMT_YELLOW, FMT_LIT("]]"));
	};

	if (truncated_rows) {
		if (head_rows > 0) FMT(fmt, FMT_REPEAT(' ', offset));
		FMT(fmt, FMT_YELLOW, FMT_LIT(" ..."));
		if (tail_rows > 0) FMT(fmt, FMT_LIT("\n"));
	};

	for (u64 i = mat->d0 - tail_rows; i < mat->d0; i += 1) {
		FMT(fmt, FMT_REPEAT(' ', offset));
		FMT(fmt, FMT_YELLOW, FMT_LIT(" ["));

		for (u64 j = 0; j < head_cols; j += 1) {
			f32 x = mat->ptr[i * mat->s0 + j * mat->s1];
			item_fmt_item(fmt, x, opt->style);
			if (j + 1 < head_cols || truncated_cols || tail_cols > 0) FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
		};

		if (truncated_cols) FMT(fmt, FMT_YELLOW, FMT_LIT("..., "));

		for (u64 j = mat->d1 - tail_cols; j < mat->d1; j += 1) {
			f32 x = mat->ptr[i * mat->s0 + j * mat->s1];
			item_fmt_item(fmt, x, opt->style);
			if (j + 1 < mat->d1) FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
		};

		if (i + 1 < mat->d0) FMT(fmt, FMT_YELLOW, FMT_LIT("],\n"));
		else FMT(fmt, FMT_YELLOW, FMT_LIT("]]"));
	};

	return true;
};

// b8 matrix_fmt_raw(Fmt * fmt, Matrix const * mat, u8 depth, TenFmtOpt const * opt) {
// 	u32 const offset = depth * DEPTH_PADDING;
//
// 	if (mat->d0 == 0 || mat->d1 == 0) {
// 		FMT(fmt, FMT_YELLOW, FMT_LIT("[]\n"));
// 		return true;
// 	};
//
// 	u64 const d0_max = MIN(mat->d0, opt->d0);
// 	u64 const d1_max = MIN(mat->d1, opt->d1) - 1;
// 	b8  const newline = opt->line < d0_max;
//
// 	for (u64 i = 0; i < d0_max; i += 1) {
// 		Str const beg = (i == 0) ? S("[[") : S(" [");
// 		Str const end = (i + 1 == d0_max)
// 			? S("]]")
// 			: (newline) ? S("],\n") : S("],");
//
// 		if (newline) {
// 			u32 repeat = (i == 0) ? 0 : offset;
// 			FMT(fmt, FMT_REPEAT(' ', repeat));
// 		};
//
// 		FMT(fmt, FMT_YELLOW, FMT_STR(beg));
//
// 		for (u64 j = 0; j < d1_max; j += 1) {
// 			f32 x = mat->ptr[i * mat->s0 + j * mat->s1];
// 			item_fmt_item(fmt, x, opt->style);
// 			FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
// 		};
//
// 		if (d1_max < mat->d1 - 1) {
// 			FMT(fmt, FMT_YELLOW, FMT_LIT("..."));
// 		} else {
// 			f32 x = mat->ptr[i * mat->s0 + d1_max * mat->s1];
// 			item_fmt_item(fmt, x, opt->style);
// 		};
//
// 		FMT(fmt, FMT_YELLOW);
// 		if (i == d0_max - 1 && d0_max < mat->d0) {
// 			FMT(fmt, FMT_LIT("], ...]"));
// 		} else {
// 			FMT(fmt, FMT_STR(end));
// 		};
// 	};
//
// 	return true;
// };

b8 ten_fmt_rec(Fmt * fmt, Tensor ten, u8 depth, TenFmtOpt const * opt) {
	if (ten.dimension == 0) {
		item_fmt_item(fmt, ten.ptr[0], opt->style);
		FMT(fmt, FMT_LIT("\n"));
		return true;
	};

	if (ten.dimension == 1) {
		vector_fmt_raw(fmt, &ten, opt);
		if (depth == 0) FMT(fmt, FMT_LIT("\n"));
		return true;
	};

	if (ten.dimension == 2) {
		Matrix mat = (Matrix){
			.ptr = ten.ptr + ten.offset,
			.d0 = ten.ds[0],
			.d1 = (ten.dimension == 2) ? ten.ds[1] : 1,
			.s0 = ten.strides[0],
			.s1 = (ten.dimension == 2) ? ten.strides[1] : 1,
		};
		matrix_fmt_raw(fmt, &mat, depth, opt);
		if (depth == 0) FMT(fmt, FMT_LIT("\n"));
		return true;
	}

	u64 const d0_max = CLAMP(opt->params / ten.strides[0], 1ull, ten.ds[0]);

	for (u32 i = 0; i < d0_max; i += 1) {
		if (i == 0) {
			FMT(fmt, FMT_YELLOW, FMT_LIT("["));
		} else {
			FMT(fmt, FMT_REPEAT(' ', depth + 1));
		};

		Tensor inner = {0}; TenSlice slice = TEN_SLICE(DIM_INDEX(i));
		if (!ten_slice(&ten, &inner, &slice)) return false;
		ten_fmt_rec(fmt, inner, depth + 1, opt);

		if (i + 1 != d0_max) FMT(fmt, FMT_REPEAT('\n', ten.dimension - 1));
	};

	if (d0_max < ten.ds[0]) {
		FMT(fmt, FMT_YELLOW, FMT_LIT(", ...]"));
	} else {
		FMT(fmt, FMT_YELLOW, FMT_LIT("]"));
	};

	if (depth == 0) FMT(fmt, FMT_LIT("\n"));
	return true;
};

b8 ten_fmt_ex(Fmt * fmt, Tensor const * ten, TenFmtOpt opt) {
	u32 d0 = 0; u32 d1 = 0;
	if (ten->dimension >= 2) {
		d0 = ten->ds[ten->dimension - 2];
		d1 = ten->ds[ten->dimension - 1];
	} else if (ten->dimension == 1) {
		d0 = ten->ds[0];
		d1 = 1;
	};

	ten_fmt_opt_restore(&opt, d0, d1);

	if (str_sane(opt.name)) {
		FMT(fmt,
			FMT_CYAN, FMT_STR(opt.name),
			FMT_YELLOW, FMT_LIT(" = "),
			FMT_LIT(" "),
		);
	};

	return ten_fmt_rec(fmt, *ten, 0, &opt);
};

// |================================================================================================|
// |> TENSOR MATH                                                                                   |

// AxBxC * scalar -> AxBxC
//   AxB *   B    -> A
//   AxB *   BxC  -> AxC
// KxAxB *   BxC  -> KxAxC
//   AxB * MxBxC  -> ???
// KxAxB * MxBxC  -> ???
TenCode ten_mul(Tensor * d, Tensor const * a, Tensor const * b) {
	
};

// AxBxC + scalar -> AxBxC
// AxBxC +     C  -> AxBxC
// AxBxC +   BxC  -> AxBxC
// AxBxC + AxBxC  -> AxBxC
TenCode ten_add(Tensor * d, Tensor const * a, Tensor const * b) {
	if (d->dimension != a->dimension) return TEN_CODE_DISMATCH_OUTPUT;
	if (a->dimension < b->dimension) return TEN_CODE_DISMATCH_COUNT;

	for (u32 i = 0; i < d->dimension; i += 1) {
		if (d->ds[i] != a->ds[i]) return TEN_CODE_DISMATCH_OUTPUT;
	};

	u32 const offset = a->dimension - b->dimension;
	for (u32 i = 0; i < b->dimension; i += 1) {
		if (a->ds[i + offset] != b->ds[i]) return TEN_CODE_DISMATCH_OUTPUT;
	};

	// u32 chunks = 1;
	// for (u32 i = 0; i < offset; i += 1) chunks *= a->ds[i];
	//
	// u32 item = 1;
	// for (u32 i = 0; i < b->dimension; i += 1) item *= b->ds[i];
	//
	// for (u32 i = 0; i < chunks; i += 1) {
	// 	for (u32 j = 0; j < item; j += 1) {
	// 		d.ptr[0 + i + j] = a.ptr[0 + i + j] + b.ptr[j];
	// 	};
	// };
};

// LATER:
// TenCode ten_fma(Tensor * d, ...);

// AxBxC - scalar -> AxBxC
// AxBxC -     C  -> AxBxC
// AxBxC -   BxC  -> AxBxC
// AxBxC - AxBxC  -> AxBxC
TenCode ten_sub(Tensor * d, Tensor const * a, Tensor const * b) {
	
};

TenCode ten_leaky_relu(Tensor * d, Tensor const * a, f32 c);


