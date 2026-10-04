#ifndef RK_MIR_CORE_H
#define RK_MIR_CORE_H

#include "std/core.h"

// indet: u24, tag: u8
typedef u32 MirIndex;

enum: u32 {
	MIR_INDEX_NIL,
};

// |================================================================================================|
// |> MIR TYPES                                                                                     |

typedef u32 MirTy;

enum: u32 {
	// error - cannot infer type
	MIR_TY_NIL,
	// just not infer type - not error
	MIR_TY_POISON,
	// f16, f32, f64, f80, f128
	MIR_TY_FLOAT,
	// i8, i16, i32, i64, i128
	MIR_TY_SIGNED,
	// u8, u16, u32, u64, u128
	MIR_TY_UNSIGNED,
	// []T, [:0]T, [N:0]T, [?:0]T,
	MIR_TY_ARRAY,
	// T^mut
	MIR_TY_REF,
	// struct { a: A, b: B, ... }
	MIR_TY_STRUCT,
	// enum { a, b, ... }
	MIR_TY_ENUM,
	#define MIR_TY_END (MIR_TY_ENUM + 1)
};

typedef struct {} MirTyPoison;

typedef struct { u16 bits; } MirTyFloat;
typedef struct { u16 bits; } MirTySigned;
typedef struct { u16 bits; } MirTyUnsigned;

typedef struct {
	MirIndex count;
	MirTy ty;
} MirTyArray;

typedef enum: u8 {
	MIR_TAG_POISON,
	MIR_TAG_NUMBER,
	MIR_TAG_IDENT,
} MirTag;

#define MIR_NODE_HEAD MirTy ty; MirTag tag

typedef struct { MIR_NODE_HEAD; } MirNodeHead;

typedef struct {
	MIR_NODE_HEAD;

} MirNodeNum;

typedef struct {
	u64 count;
	MirTy kind;
} MirTySlice;

#endif // RK_MIR_CORE_H
