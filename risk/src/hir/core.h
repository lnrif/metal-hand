#ifndef RK_HIR_CORE_H
#define RK_HIR_CORE_H

#include "risk/src/ast/ast.h"
#include "std/mem/list.h"

// |================================================================================================|
// |> HIR INDEX                                                                                     |
// |================================================================================================|

/*

typedef u32 HirIdx;

enum: u32 { HIR_IDX_NIL };

typedef enum: u8 {
	// %n = poison
	// bad ast, check [AST_T_POISON]
	HIR_TAG_POISON,

	// %n = unit
	// `a := ()`;
	HIR_TAG_UNIT,
	// %n = number(x)
	// `69`, `1984`, ...
	HIR_TAG_NUMBER,
	// %n = ident(x)
	// `variable`, `count`, ...
	HIR_TAG_NAME,
	// %n = type(x)
	// `u32`, `struct {}`, `(...) -> (...)`
	HIR_TAG_TYPE,

	// %n = implies(x, y)
	// `x => y`
	HIR_TAG_IMPLIES,
	// %n = coerce(expr, type)
	// `@as(type, expr)`, `x: T = E` -> `x := @as(E, T)`
	HIR_TAG_COERCE,

	// %n = add(%a, %b)
	// `a + b`
	HIR_TAG_ADD,
	// %n = sub(%a, %b)
	// `a - b`
	HIR_TAG_SUB,
	// %n = mul(%a, %b)
	// `a * b`
	HIR_TAG_MUL,
	// %n = div(%a, %b)
	// `a / b`
	HIR_TAG_DIV,

	// %n = eq(%a, %b)
	// `a == b`
	HIR_TAG_EQ,
	// %n = ne(%a, %b)
	// `a != b`
	HIR_TAG_NE,

	// %n = lt(%a, %b)
	// `a < b`
	HIR_TAG_LT,
	// %n = le(%a, %b)
	// `a <= b`
	HIR_TAG_LE,
	// %n = gt(%a, %b)
	// `a > b`
	HIR_TAG_GT,
	// %n = ge(%a, %b)
	// `a >= b`
	HIR_TAG_GE,

	// %n = alloc(%t)
	// `x'mut: T`
	HIR_TAG_ALLOC,
	// %n = load(%x)
	// `x`
	HIR_TAG_LOAD,
	// %n = store(%x, %v)
	// `x = v`
	HIR_TAG_STORE,

	// %n = ret(%label, %x)
	// `ret'label x`
	HIR_TAG_RET,
} HirTag;

typedef struct {
	union {
		
	} as;
	HirTag tag;
} HirInst;

 */

// |================================================================================================|
// |> HIR BLOCK                                                                                     |
// |================================================================================================|

/*

typedef LIST_TYPE(HirInst) HirInstList;

typedef struct {
	HirInstList insts;
} HirBlock;

*/

// |================================================================================================|
// |> HIR TYPES                                                                                     |
// |================================================================================================|

// typedef u32 HirTy;
//
// enum: u32 {
// 	// error - cannot infer type
// 	HIR_TY_NIL,
// 	// just not infer type - not error
// 	HIR_TY_POISON,
// 	// i8, i16, i32, i64, i128
// 	HIR_TY_SIGNED,
// 	// u8, u16, u32, u64, u128
// 	HIR_TY_UNSIGNED,
// 	// // f16, f32, f64, f80, f128
// 	// HIR_TY_FLOAT,
// 	// []T, [:0]T, [N:0]T, [?:0]T,
// 	HIR_TY_ARRAY,
// 	// // &T
// 	// HIR_TY_REF,
// 	// // struct { a: A, b: B, ... }
// 	// HIR_TY_STRUCT,
// 	// // enum { a, b, ... }
// 	// HIR_TY_ENUM,
// 	// // type { ... }
// 	// HIR_TY_TYPE,
// 	// ...
// 	HIR_TY_END,
// };
//
// // |================================================================================================|
// // |> HIR TYPES BASIC                                                                               |
//
// typedef struct {} HirTyPoison;
//
// typedef struct { u16 bits; } HirTyFloat;
// typedef struct { u16 bits; } HirTySigned;
// typedef struct { u16 bits; } HirTyUnsigned;
//
// // |================================================================================================|
// // |> HIR TYPES ARRAY                                                                               |
//
// typedef enum: u8 {
// 	// [~]T
// 	HIR_TY_ARR_K_DYN,
// 	// [N]T
// 	HIR_TY_ARR_K_KNOWN,
// 	// [?]T
// 	HIR_TY_ARR_K_UNKNOWN,
// } HirTyArrKind;
//
// typedef struct {
// 	// HirIndex count;
// 	HirTyArrKind kind;
// 	HirTy ty;
// } HirTyArr;
//
// #define HIR_NODE_HEAD HirTy ty; HirTag tag
//
// typedef struct { HIR_NODE_HEAD; } HirNodeHead;
//
// typedef struct {
// 	HIR_NODE_HEAD;
//
// } HirNodeNum;

// typedef enum: u8 {
// 	HIR_TY_TAG_,
// } HirTyTag;
//
// typedef struct {
// 	union {
//
// 	};
//
// 	HirTyTag tag;
// } HirTy;
//
// |================================================================================================|
// |> HIR IMPL                                                                                      |

// typedef LIST(HirTy) HirTyList;
//
// typedef struct {
// 	HirTyList types;
// } HirState;
//
// void hir_from_ast(Ast ast) {
//
// };
#endif // RK_HIR_CORE_H
