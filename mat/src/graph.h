#ifndef GRAPH_H
#define GRAPH_H

#include "ten.h"

// b8 input_init(B1 * b, TenMeta * meta);

#define BIND_EMBED(b, v, g) \
	union { \
		struct { Tensor v; Tensor g; }; \
		Bind b; \
	}

typedef struct { Tensor w; Tensor m; Tensor v; } Node;
typedef struct { Tensor w; Tensor g; } Bind;

typedef struct {
	struct { Node * ptr; u32 len; u32 cap; } nodes;
	struct { Bind * ptr; u32 len; u32 cap; } binds;
	// TODO: reallocations
} Graph;

typedef u32 NodeId;
typedef u32 BindId;

typedef struct {
	BindId x; BindId y;
	NodeId w; NodeId b;
} Linear;

typedef struct {
	BindId x; BindId y;
	NodeId a;
} Leaky;

typedef struct {
	BindId a; BindId b; BindId c;
} Add;

// 2x3 * 3x1 + 2x1 -> 2x1
// 2x1 -= 2x1
// 2x1 * 3x1^T -> 2x3
// 2x3^T *:2x1 -> 3x1
//
// [[w, w, w], [w, w, w]]
// [[x], [x], [x]]
// [[b], [b]]
// [[y], [y]]
// []

// b8 lin_init(
// 	Lin * lin,
// 	B1 x, u32 out,
// 	TenMeta * meta
// );
//
// b8 lin_fmt(Fmt * fmt, Lin const * lin);
//
// b8 lin_rnd(Lin * lin, u32 * seed);
// b8 lin_eval(Lin * lin);
// b8 lin_back(Lin * lin);
// b8 lin_learn(Lin * lin);
//
// typedef struct {
// 	B1_EMBED(xb, x, xg);
// 	B1_EMBED(yb, y, yg);
// 	f32 a; TenMeta * meta;
// } Relu;
//
// b8 relu_init(
// 	Relu * relu,
// 	B1 x, f32 a,
// 	TenMeta * meta
// );
//
// b8 relu_eval(Relu * relu);
// b8 relu_back(Relu * relu);
// b8 relu_learn(Relu * relu);
//
// typedef struct {
// 	T1 x; T1 y; T1 g;
// 	B1_EMBED(ob, o, og);
// } Add;
//
// b8 add_eval(Add * add);
// b8 add_learn(Add * add);
// b8 add_step(Add * add);
//
// |================================================================================================|
// |> GRAPH OF TENSOR'S                                                                             |

// typedef enum: u8 {
// 	INST_TAG_LINEAR,
// 	INST_TAG_RELU,
// 	INST_TAG_SUM,
// } InstTag;
//
// typedef struct {
// 	union {
// 		Lin lin;
// 		Add add;
// 		Relu relu;
// 	} as;
// 	InstTag tag;
// } Inst;
//
// typedef struct {
// 	RegMan list; Inst * ptr; u32 len; u32 cap;
// 	RegMan pool;
// } Insts;
//
// Insts insts_from_slice(Inst * ptr, u32 len);
// b8    insts_push(Insts * insts, Inst item);
// void  insts_reset(Insts * insts);
//
// b8 graph_mul(
// 	Tensor * out,
// 	Tensor const * x,
// 	Tensor const * w
// );
//
// b8 ten_add(
// 	Tensor * out,
// 	Tensor const * x,
// 	Tensor const * w
// );
//
// b8 ten_sub(
// 	Tensor * out,
// 	Tensor const * x,
// 	Tensor const * w
// );

#endif // !GRAPH_H
