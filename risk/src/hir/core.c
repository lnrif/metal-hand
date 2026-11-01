#include "risk/src/hir/core.h"
#include "risk/src/ast/ast.h"
#include "risk/src/src/core.h"
#include "std/flow/core.h"
#include "std/mem/list.h"
#include "std/mem/heap.h"

// |================================================================================================|
// |> IDS                                                                                           |

typedef u32 HirSrcId;
typedef u32 HirValId;
typedef u32 HirDecId;
typedef u32 HirTyId;

#define HIR_SRC_ID_NIL ((HirSrcId)0)
#define HIR_VAL_ID_NIL ((HirValId)0)
#define HIR_DEC_ID_NIL ((HirDecId)0)
#define HIR_TY_ID_NIL  ((HirTyId)0)

// |================================================================================================|
// |> TYPE                                                                                          |

// bool: @bytes(64) @align(64) b1;

typedef enum: u8 {
	// literally type
	HIR_TY_TAG_TYPE,
	// // uptr
	// HIR_TY_TAG_UNSIGNED_POINTER,
	// uX
	HIR_TY_TAG_UNSIGNED,
	// iX
	HIR_TY_TAG_SIGNED,
	// bX; bool (b8)
	HIR_TY_TAG_BOOLEAN,
	// (...) -> (...)
	HIR_TY_TAG_FUNCTION,
} HirTyTag;

typedef struct {
	HirDecId in;  u32 in_len;
	HirDecId out; u32 out_len;
} HirTyFun;

typedef struct {
	u64 align;
	u64 bytes;
	u64 bits;
} HirTyRepr;

typedef struct {
	union {
		struct { u32 bits; } u; // unsigned
		struct { u32 bits; } i; // signed
		struct { u32 bits; } b; // boolean
		HirTyFun             f;
	} as;
	HirTyRepr repr;
	HirTyTag tag;
} HirTy;

// |================================================================================================|
// |> INSTRUCTION                                                                                   |

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


// |================================================================================================|
// |> VALUE                                                                                         |

typedef enum: u8 {
	HIR_VAL_TAG_UNSIGNED,
	HIR_VAL_TAG_SIGNED,
	HIR_VAL_TAG_BOOLEAN,
	HIR_VAL_TAG_TYPE,
} HirValKind;

typedef struct {
	union {
		u64 u; i64 i; b64 b;
		HirTyId t;
	} as;
	struct { HirSrcId src; u32 pos; u32 len; } from;
	HirValKind kind;
} HirVal;

// |================================================================================================|
// |> DECLARATION                                                                                   |

typedef enum: u8 {
	HIR_DEC_MUT_NO,
	HIR_DEC_MUT_YES,
} HirDecMut;

typedef struct {
	HirSrcId src; u32 pos; u32 len;
	HirDecMut mut; HirTyId ty; HirValId val;
} HirDec;



// |================================================================================================|
// |> HIR STATE                                                                                     |

typedef LIST(Src)    HirSrcList;
typedef LIST(HirVal) HirValList;
typedef LIST(HirDec) HirDecList;
typedef LIST(HirTy)  HirTyList;

typedef struct { Str str; } HirName;
typedef LIST(HirName) HirNameStack;

typedef struct {
	Heap heap;
	HirSrcList srcs;
	HirDecList decs;
	HirValList vals;
	HirTyList  tys;
	HirNameStack names;
} HirState;

// |================================================================================================|
// |> HIR                                                                                           |

typedef struct {
	// ...
} Hir;

HirValId hir_eval(HirState * state, AstIdx idx);
Hir hir_from_ast(Ast ast);

HirValId hir_eval(HirState * state, AstIdx idx) {
	
};

Hir hir_from_ast(Ast ast) {
	
	PANIC("");
};
