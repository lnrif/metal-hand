
#ifndef TEN_H
#define TEN_H

#include "std/core.h"
#include "std/str/core.h"
#include "std/fmt/core.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> f32                                                                                           |

f32 f32_abs(f32 x);
f32 f32_sign(f32 x);

#define TEN_DIM_MAX 7

// |================================================================================================|
// |> TENSOR DIMENSIONS                                                                             |

typedef struct {
	u32 count;
	u32 arr[TEN_DIM_MAX];
} TenDims;

#define TEN_DIMS(dims...) \
	((TenDims){.count = LEN((u32[]){dims}), .arr = {dims}})

u32 ten_dims_size(TenDims dims);

typedef struct {
	f32 * ptr; u32 offset;
	u32 strides[TEN_DIM_MAX];

	union {
		TenDims dims;
		struct { u32 dimension; u32 ds[TEN_DIM_MAX]; };
	};
} Tensor;

// |================================================================================================|
// |> TENSOR SLICING                                                                                |

typedef struct { u32 beg; u32 end; u32 step; } DimSlice;

#define DIM_END U32_MAX
#define DIM_SLICE(_beg, _end, _step...) ((DimSlice){.beg = (_beg), .end = (_end), _step})
#define DIM_INDEX(idx) DIM_SLICE(idx, DIM_END, .step = DIM_END)
#define DIM_FULL(_step...) DIM_SLICE(0, DIM_END, _step)

typedef struct {
	u32 count;
	DimSlice ds[TEN_DIM_MAX];
} TenSlice;

Tensor ten_slice_ex(Tensor * ten, TenSlice slice);

#define ten_slice(ten, slices...) \
	ten_slice_ex(ten, (TenSlice){ \
		.count = LEN((DimSlice[]){slices}), \
		.ds = {slices}, \
	})

// |================================================================================================|
// |> TENSOR API                                                                                    |

b8 ten_init(Tensor * ten, TenDims dims, RegMan man);

// |================================================================================================|
// |> TENSOR FORMAT                                                                                 |

typedef struct { f32 min; f32 max; } TenRange;
typedef struct { u32 d1; u32 d2; u32 line; } TenLimit;

#define TEN_STYLE_EMBED \
	union { \
		TenLimit limit; \
		struct { u32 d1; u32 d2; u32 line; }; \
	}; \
	\
	union { \
		TenRange range; \
		struct { f32 min; f32 max; }; \
	}; \
	\
	u8 prec; \

typedef struct { TEN_STYLE_EMBED; } TenStyle;

typedef struct {
	Str name;
	union {
		TenStyle style;
		struct { TEN_STYLE_EMBED; };
	};
} TenFmtOpt;

b8 ten_fmt_dims_ex(Fmt * fmt, Tensor const * ten, TenFmtOpt opt);
#define ten_fmt_dims(fmt, ten, opt...) ten_fmt_dims_ex(fmt, ten, (TenFmtOpt){opt})

b8 ten_fmt(Fmt * fmt, Tensor const * ten, TenFmtOpt opt);
#define ten_fmt(fmt, ten, opt...) ten_fmt_dims_ex(fmt, ten, (TenFmtOpt){opt})

// |================================================================================================|
// |> Tenzor                                                                                        |

typedef enum: u8 {
	TEN_LEARN_NO,
	TEN_LEARN_YES,
} TenLearn;

typedef struct {
	RegMan man;
	TenLearn learn;
	f32 step;
} TenMeta;

typedef struct { f32 * ptr; u32 len; } T1;
typedef struct { f32 * ptr; u32 d1, d2; } T2;

#define T1_ON_STACK(_len) \
	((T1){.ptr = (f32[(_len)]){0.0}, .len = (_len)})

#define T2_ON_STACK(_row, _t1) \
	((T2){.ptr = (f32[(_row) * (_t1)]){0.0}, .row = (_row), .t1 = (_t1)})

#define T2_FROM_T1(_t1) \
	((T2){.ptr = (_t1).ptr, .d1 = (_t1).len, .d2 = 1})

b8 t1_alloc(T1 * col, RegMan man, u32 len);
b8 t2_alloc(T2 * mat, RegMan man, u32 row, u32 col);

void t1_rnd(T1 c, u32 * seed);
void t2_rnd(T2 m, u32 * seed);

b8 t2_fma_t1(T1 dst, T2 w, T1 x, T1 b);

// b8 t1_add(T1 dst, T1 a, T1 b);
// b8 t1_sub(T1 dst, T1 a, T1 b);
// b8 t1_mul(T1 dst, T1 a, T1 b);
// b8 t1_scale(T1 dst, T1 a, f32 scale);
b8 t2_fmt_ex(Fmt * fmt, T2 t2, TenFmtOpt opt);
#define t2_fmt(fmt, t2, opt...) t2_fmt_ex(fmt, t2, (TenFmtOpt){opt})
#define t1_fmt(fmt, t1, opt...) t2_fmt(fmt, T2_FROM_T1(t1), opt)

#define FMT_T1(t1) FMT_T2(T2_FROM_T1(t1))
#define FMT_T2(t2) FMT_U64((t2).d1), FMT_LIT("x"), FMT_U64(t2.d2)

// y := wx + b;
// g := ...; // y - e;

// xg += g * w;
// wg += g * x;
// bg += g;

// x -= xg;
// w -= wg;
// b -= bg;

typedef struct {
	T1 v; T1 g;
} B1;

// b8 input_init(B1 * b, TenMeta * meta);

#define B1_EMBED(b, v, g) \
	union { \
		struct { T1 v; T1 g; }; \
		B1 b; \
	}

typedef struct {
	B1_EMBED(xb, x, xg);
	B1_EMBED(yb, y, yg);
	T2 w; T2 wg;
	T1 b; T1 bg;
	TenMeta * meta;
} Lin;

b8 lin_init(
	Lin * lin,
	B1 x, u32 out,
	TenMeta * meta
);

b8 lin_fmt(Fmt * fmt, Lin const * lin);

b8 lin_rnd(Lin * lin, u32 * seed);
b8 lin_eval(Lin * lin);
b8 lin_back(Lin * lin);
b8 lin_learn(Lin * lin);

typedef struct {
	B1_EMBED(xb, x, xg);
	B1_EMBED(yb, y, yg);
	f32 a; TenMeta * meta;
} Relu;

b8 relu_init(
	Relu * relu,
	B1 x, f32 a,
	TenMeta * meta
);

b8 relu_eval(Relu * relu);
b8 relu_back(Relu * relu);
b8 relu_learn(Relu * relu);

typedef struct {
	T1 x; T1 y; T1 g;
	B1_EMBED(ob, o, og);
} Add;

b8 add_eval(Add * add);
b8 add_learn(Add * add);
b8 add_step(Add * add);

typedef enum: u8 {
	INST_TAG_LINEAR,
	INST_TAG_RELU,
	INST_TAG_SUM,
} InstTag;

typedef struct {
	union {
		Lin lin;
		Add add;
		Relu relu;
	} as;
	InstTag tag;
} Inst;

typedef struct {
	RegMan list; Inst * ptr; u32 len; u32 cap;
	RegMan pool;
} Insts;

Insts insts_from_slice(Inst * ptr, u32 len);
b8    insts_push(Insts * insts, Inst item);
void  insts_reset(Insts * insts);

#endif // !TEN_H
