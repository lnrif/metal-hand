#ifndef MAT_H
#define MAT_H

#include "std/core.h"
#include "std/str/core.h"
#include "std/fmt/core.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> Matrix                                                                                        |

typedef struct {
	f32 * ptr ALIGNED(4);
	u64 row, col;
} Mat;

#define MAT_ON_STACK(_row, _col) \
	((Mat){.ptr = (f32[(_row) * (_col)]){0.0}, .row = (_row), .col = (_col)})

#define COL_ON_STACK(_len) \
	((Col){.ptr = (f32[(_len)]){0.0}, .len = (_len)})

typedef struct { f32 * ptr; u64 len; } Col;

#define MAT_FROM_COL(_col) \
	((Mat){.ptr = (_col).ptr, .row = (_col).len, .col = 1})

b8 mat_fma_col(Col dst, Mat w, Col x, Col b);

// b8 col_add(Col dst, Col a, Col b);
// b8 col_sub(Col dst, Col a, Col b);
// b8 col_mul(Col dst, Col a, Col b);
// b8 col_scale(Col dst, Col a, f32 scale);

typedef struct { f32 min; f32 max; } MatRange;

typedef struct {
	Str name;
	u8 prec;
	MatRange range;
} MatStyle;

b8 mat_fmt_ex(Fmt * fmt, Mat mat, MatStyle style);
#define mat_fmt(fmt, mat, style...) mat_fmt_ex(fmt, mat, (MatStyle){style})

// y := wx + b;
// g := ...; // y - e;

// xg += g * w;
// wg += g * x;
// bg += g;

// x -= xg;
// w -= wg;
// b -= bg;

typedef struct {
	Col v; Col g;
} Bind;

#define BIND_EMBED(b, v, g) \
	union { \
		struct { Col v; Col g; }; \
		Bind b; \
	}

typedef struct {
	BIND_EMBED(xb, x, xg);
	BIND_EMBED(yb, y, yg);
	Mat w; Mat wg;
	Col b; Col bg;
} Lin;

b8 lin_eval(Lin * lin);
b8 lin_learn(Lin * lin);
b8 lin_step(Lin * lin);

typedef struct {
	BIND_EMBED(xb, x, xg);
	BIND_EMBED(yb, y, yg);
	f32 a;
} Relu;

b8 relu_eval(Relu * relu);
b8 relu_learn(Relu * relu);
b8 relu_step(Relu * relu);

typedef struct {
	Col x; Col y; Col g;
	BIND_EMBED(ob, o, og);
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

#endif // !MAT_H
