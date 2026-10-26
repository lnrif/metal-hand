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

// |================================================================================================|
// |> TENSOR DIMENSIONS                                                                             |

#define TEN_DIM_MAX 3

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

void dim_slice_restore(DimSlice * slice);

#define DIM_END U32_MAX
#define DIM_SLICE(_beg, _end, _step...) ((DimSlice){.beg = (_beg), .end = (_end), _step})
#define DIM_INDEX(idx) DIM_SLICE(idx, DIM_END, .step = DIM_END)
#define DIM_FULL(_step...) DIM_SLICE(0, DIM_END, _step)

typedef enum: u8 {
	TEN_SLICE_OK,
	TEN_SLICE_INDEX_OUT_OF_BOUNDS,
	TEN_SLICE_SLICE_OUT_OF_BOUNDS,
} TenSliceKind;

typedef struct {
	u32 count;
	DimSlice ds[TEN_DIM_MAX];
	u32 illegal; TenSliceKind kind;
} TenSlice;

#define TEN_SLICE(dims...) \
	((TenSlice){ \
		.count = LEN((DimSlice[]){dims}), .ds = {dims}, \
		.illegal = 0, .kind = TEN_SLICE_OK, \
	})

b8 ten_slice(Tensor const * in, Tensor * out, TenSlice * slice);

// |================================================================================================|
// |> TENSOR INIT                                                                                   |

b8 ten_init(Tensor * ten, TenDims dims, RegMan man);

// |================================================================================================|
// |> TENSOR FORMAT                                                                                 |

typedef struct { f32 min; f32 max; } TenRange;
typedef struct { u32 d0; u32 d1; u32 line; u32 params; } TenLimit;

#define TEN_STYLE_EMBED \
	union { \
		TenLimit limit; \
		struct { u32 d0; u32 d1; u32 line; u32 params; }; \
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

b8 ten_fmt_ex(Fmt * fmt, Tensor const * ten, TenFmtOpt opt);
#define ten_fmt(fmt, ten, opt...) ten_fmt_ex(fmt, ten, (TenFmtOpt){opt})

// |================================================================================================|
// |> TENSOR MATH                                                                                   |

typedef enum: u8 {
	TEN_CODE_OK,
	TEN_CODE_DISMATCH_OUTPUT,
	TEN_CODE_DISMATCH_INPUT,
	TEN_CODE_DISMATCH_COUNT,
} TenCode;

// AxBxC * scalar -> AxBxC
//   AxB *   B    -> A
//   AxB *   BxC  -> AxC
// KxAxB *   BxC  -> KxAxC
//   AxB * MxBxC  -> ???
// KxAxB * MxBxC  -> ???
TenCode ten_mul(Tensor * d, Tensor const * a, Tensor const * b);

// AxBxC + scalar -> AxBxC
// AxBxC +     C  -> AxBxC
// AxBxC +   BxC  -> AxBxC
// AxBxC + AxBxC  -> AxBxC
TenCode ten_add(Tensor * d, Tensor const * a, Tensor const * b);

// LATER:
// TenCode ten_fma(Tensor * d, ...);

// AxBxC - scalar -> AxBxC
// AxBxC -     C  -> AxBxC
// AxBxC -   BxC  -> AxBxC
// AxBxC - AxBxC  -> AxBxC
TenCode ten_sub(Tensor * d, Tensor const * a, Tensor const * b);

TenCode ten_leaky_relu(Tensor * d, Tensor const * a, f32 c);

#endif // !TEN_H
