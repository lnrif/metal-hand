#ifndef RK_AST_H
#define RK_AST_H

#include "std/str/core.h"
#include "std/mem/page.h"
#include "std/mem/bump.h"
#include "risk/src/src/core.h"

// |================================================================================================|
// |> AST TAG                                                                                       |

typedef enum: u8 {
	AST_T_ATOM_BEG,
		// dummy node
		AST_T_POISON = AST_T_ATOM_BEG,
		// ident
		AST_T_ATOM_IDENT,
		// 100
		AST_T_ATOM_NUMBER,
		// "text"
		AST_T_ATOM_STRING,
	AST_T_ATOM_END,

	AST_T_UNA_BEG = AST_T_ATOM_END,
		// +a
		AST_T_UNA_POS = AST_T_UNA_BEG,
		// -a
		AST_T_UNA_NEG,
		// !a
		AST_T_UNA_NOT,
		// .a
		AST_T_UNA_DOT,
		// a.*
		AST_T_UNA_DEREF,
		// a.&
		AST_T_UNA_REF,
	AST_T_UNA_END,

	AST_T_BIN_BEG = AST_T_UNA_END,
		// a + b
		AST_T_BIN_ADD = AST_T_BIN_BEG,
		// a - b
		AST_T_BIN_SUB,
		// a * b
		AST_T_BIN_MUL,
		// a / b
		AST_T_BIN_DIV,

		// a += b
		AST_T_BIN_ADD_SET,
		// a -= b
		AST_T_BIN_SUB_SET,
		// a *= b
		AST_T_BIN_MUL_SET,
		// a /= b
		AST_T_BIN_DIV_SET,

		// a < b
		AST_T_BIN_LT,
		// a <= b
		AST_T_BIN_LE,
		// a > b
		AST_T_BIN_GT,
		// a >= b
		AST_T_BIN_GE,
		// a == b
		AST_T_BIN_EQ,
		// a != b
		AST_T_BIN_NEQ,

		// a | b
		AST_T_BIN_OR,
		// a & b
		AST_T_BIN_AND,

		// a || b
		AST_T_BIN_LOG_OR,
		// a && b
		AST_T_BIN_LOG_AND,

		// a.b
		AST_T_BIN_DOT,
		// a'b
		AST_T_BIN_TICK,

		// a: b
		AST_T_BIN_COLON,
		// a = b
		AST_T_BIN_SET,
		// a := b
		AST_T_BIN_DEF,

		// a => b
		AST_T_BIN_IMPLIES,
		// a -> b
		AST_T_BIN_ARROW,
		// a :: b
		AST_T_BIN_EXTEND,
		// a <| b
		AST_T_BIN_APPLY,
	AST_T_BIN_END,

	// if a then b else c;
	AST_T_IF_ELSE = AST_T_BIN_END,

	// (a, b, ...) / (a; b; ...)
	// [a, b, ...] / [a; b; ...]
	// {a, b, ...} / {a; b; ...}
	AST_T_SEQ,

	// f(a, b, ...) / f(a; b; ...)
	// f[a, b, ...] / f[a; b; ...]
	// f{a, b, ...} / f{a; b; ...}
	AST_T_CALL_POSTFIX,
	// (a, b, ...)f / (a; b; ...)f
	// [a, b, ...]f / [a; b; ...]f
	// {a, b, ...}f / {a; b; ...}f
	AST_T_CALL_PREFIX,
} AstTag;

// if x > 0 then x else x * 0.05

Str ast_name(AstTag tag);
// Str ast_tag(AstTag tag);

// |================================================================================================|
// |> AST NODES                                                                                     |

// TODO: replace with [typedef u16 AstOff]
typedef u32 AstIdx;
#define AST_IDX_NIL ((AstIdx)0)

#define AST_NODE_ALIGN 4

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AstTag tag; u8 _BAD_PAD[1]; u16 len; u32 pos;
} AstAtom;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AstTag tag; u8 _BAD_PAD[1]; u16 len; u32 pos;
	AstIdx node;
} AstUna;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AstTag tag; u8 _BAD_PAD[1]; u16 len; u32 pos;
	AstIdx lhs, rhs;
} AstBin;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AstTag tag; u8 _BAD_PAD[1]; u16 len; u32 pos;
	AstIdx cond, yes, no;
} AstIfElse;

typedef enum: u8 {
	AST_D_NONE  = 0b00,
	AST_D_COMMA = 0b01,
	AST_D_SEMI  = 0b10,
} AstDelim;

typedef enum: u8 {
	AST_B_EOF     = 0b000,
	AST_B_PAREN   = 0b001,
	AST_B_BRACE   = 0b010,
	AST_B_BRACKET = 0b011,
	AST_B_POISON  = 0b100,
} AstBound;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AstTag tag; AstBound bound; AstDelim delim; u8 _BAD_PAD[1];
	u32 pos_open; u32 pos_close;
	u32 count; AstIdx idx[];
} AstSeq;

typedef union ALIGNED(AST_NODE_ALIGN) {
	AstSeq seq;
	struct {
		AstTag tag; AstBound bound; AstDelim delim; u8 _BAD_PAD[1];
		u32 pos_open; u32 pos_close;
		u32 count; AstIdx idx[];
	};
} AstCall;

// |================================================================================================|
// |> AST POOL                                                                                      |

#define AST_IDX_AS_PTR(ast, idx) (uptr)((ast)->nodes.beg + AST_NODE_ALIGN * (idx))
#define AST_PTR_AS_IDX(ast, ptr) ((AstIdx)(((uptr)ptr) - (ast)->nodes.beg) / AST_NODE_ALIGN)

typedef struct {
	Src src;
	Bump nodes;
	AstIdx root;
} Ast;

Ast ast_init(Pages * nodes, Src src);
void ast_free(Ast * ast);

void * ast_node(Ast * ast, u64 size);

#define ast_alloc(ast, node...) ({ \
	typeof(node) * _ptr = ast_node(ast, sizeof(typeof(node))); \
	if (_ptr != 0) *_ptr = node; \
	(_ptr == 0) ? AST_IDX_NIL : AST_PTR_AS_IDX(ast, _ptr); \
})

#endif // !RK_AST_H
