#ifndef RK_PRS_AST_H
#define RK_PRS_AST_H

#include "mem/mem.h"
#include "src/src.h"
#include "fmt/fmt.h"

////////////////////////////////////////////////////////////////
// kind

ENUM(AstKind, u8) {
	AST_K_POISON = 0,

	AST_K_ATOM_BEG,
		// ident
		AST_K_ATOM_IDT = AST_K_ATOM_BEG,
		// 100
		AST_K_ATOM_NUM,
		// "text"
		AST_K_ATOM_STR,
	AST_K_ATOM_END,

	AST_K_UNA_BEG = AST_K_ATOM_END,
		// +a
		AST_K_UNA_POS = AST_K_UNA_BEG,
		// -a
		AST_K_UNA_NEG,
		// !a
		AST_K_UNA_NOT,
		// .a
		AST_K_UNA_DOT,
		// a.*
		AST_K_UNA_DEREF,
		// &a
		AST_K_UNA_REF,
		// if a
		AST_K_UNA_IF,
	AST_K_UNA_END,

	AST_K_BIN_BEG = AST_K_UNA_END,
		// a + b
		AST_K_BIN_ADD = AST_K_BIN_BEG,
		// a - b
		AST_K_BIN_SUB,
		// a * b
		AST_K_BIN_MUL,
		// a / b
		AST_K_BIN_DIV,

		// a < b
		AST_K_BIN_LT,
		// a <= b
		AST_K_BIN_LE,
		// a > b
		AST_K_BIN_GT,
		// a >= b
		AST_K_BIN_GE,
		// a == b
		AST_K_BIN_EQ,
		// a != b
		AST_K_BIN_NEQ,

		// a || b
		AST_K_BIN_LOG_OR,
		// a && b
		AST_K_BIN_LOG_AND,

		// a.b
		AST_K_BIN_DOT,
		// a'b
		AST_K_BIN_TICK,

		// a: b
		AST_K_BIN_COLON,
		// a = b
		AST_K_BIN_SET,
		// a := b
		AST_K_BIN_DEF,

		// a => b
		AST_K_BIN_IMPLIES,
		// a -> b
		AST_K_BIN_ARROW,
		// a :: b
		AST_K_BIN_EXTEND,
		// a <| b
		AST_K_BIN_APPLY,

		// a then b
		AST_K_BIN_THEN,
		// a elif b
		AST_K_BIN_ELIF,
		// a else b
		AST_K_BIN_ELSE,
	AST_K_BIN_END,

	// (a, b, ...) / (a; b; ...)
	// [a, b, ...] / [a; b; ...]
	// {a, b, ...} / {a; b; ...}
	AST_K_SEQ = AST_K_BIN_END,

	// (a, b, ...) f / (a; b; ...) f
	// [a, b, ...] f / [a; b; ...] f
	// {a, b, ...} f / {a; b; ...} f
	AST_K_CALL_PREFIX,

	// f(a, b, ...) / f(a; b; ...)
	// f[a, b, ...] / f[a; b; ...]
	// f{a, b, ...} / f{a; b; ...}
	// f.{a, b, ...} / f.{a; b; ...}
	AST_K_CALL_POSTFIX,

	// a, b, c / a; b; c;
	AST_K_ROOT,

	// if ...

	// AST_K_BRACES, // expr { e1, e2; ... }
	// AST_K_PARENS, // expr(e1, e2; ...)

	// x := _'{
	//    if !cond break'_ 0;
	//    break'_ 1;
	// };

	// value: _'{
	//     x := f(...);
	//     if !x break'_ u32;
	//     break'_ u64;
	// } = 100;
};

StrZ ast_name(AstKind kind);
StrZ ast_tag(AstKind kind);

////////////////////////////////////////////////////////////////
// index

// absolute index with step of 4 (all nodes have align of 4)
// it meens that AST limited by 2^32 * 2^2 = 16GiB (fair enougn XD)
typedef u32 AstIdx;
typedef u32 AstPos;

#define AST_IDX_INVALID U32_MAX
#define AST_NODE_ALIGN 4

////////////////////////////////////////////////////////////////
// node

ENUM(AstDelim, u8) {
	AST_D_NONE = 0b00,
	AST_D_COMMA = 0b01,
	AST_D_SEMI  = 0b10,
};

ENUM(AstBound, u8) {
	AST_B_EOF     = 0b00,
	AST_B_PAREN   = 0b01,
	AST_B_BRACE   = 0b10,
	AST_B_BRACKET = 0b11,
};

typedef union ALIGNED(AST_NODE_ALIGN) {
	// u24:u8
	u32 pos_and_kind;
	struct {
		u8 _pad[3];
		AstKind kind;
	};
} AstNodeWithPos;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	u16 count;
	u8 delim_and_bound;
	AstKind kind;
} AstNodeWithSeq;

typedef union ALIGNED(AST_NODE_ALIGN) {
	// u24:u8
	u32 load_and_kind;
	union {
		AstNodeWithPos pos;
		AstNodeWithSeq seq;
	} with;
	struct {
		u8 _pad[3];
		AstKind kind;
	};
} AstNode;

////////////////////////////////
// node with pos

#define AST_NODE_WITH_POS(_kind, _pos) \
	((AstNodeWithPos){ \
		.pos_and_kind = ((u32)(_kind) << 24) | ((_pos) & 0xFFFFFFU), \
	}) \

#define AST_NODE_POS(node) ((node)->pos_and_kind & 0xFFFFFF)

////////////////////////////////
// node with sequence

#define AST_NODE_WITH_SEQ(_kind, delim, bound, _count) \
	((AstNodeWithSeq){ \
		.count = (_count), \
		.delim_and_bound = ((AstDelim)(delim) & 0xF) | (((AstBound)(bound) & 0xF) * 16), \
		.kind = (_kind), \
	}) \

#define AST_NODE_SEQ_DELIM(node) (((node)->delim_and_bound >> 0) & 0xF)
#define AST_NODE_SEQ_BOUND(node) (((node)->delim_and_bound >> 4) & 0xF)

////////////////////////////////
// poison

#define AST_POISON_ALLOC AstNodeWithPos node_pos;
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_POISON_ALLOC; } AstPoisonAlloc;
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_POISON_ALLOC; } AstPoison;

#define AST_POISON(token) \
	((AstPoisonAlloc){ \
		.node_pos = AST_NODE_WITH_POS(AST_K_POISON, token.pos), \
	}) \

////////////////////////////////
// atom

#define AST_ATOM_ALLOC AstNodeWithPos node_pos; u32 len
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_ATOM_ALLOC; } AstAtomAlloc;
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_ATOM_ALLOC; } AstAtom;

#define AST_ATOM(kind, token) \
	((AstAtomAlloc){ \
		.node_pos = AST_NODE_WITH_POS(kind, token.pos), \
		.len      = lex_len(token), \
	}) \

#define AST_PARENS(kind, open, close) \
	((AstAtomAlloc){ \
		.node_pos = AST_NODE_WITH_POS(kind, open.pos), \
		.len      = (close).pos - (open).pos + 1, \
	}) \

////////////////////////////////
// una

#define AST_UNA_ALLOC AstNodeWithPos node_pos;
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_UNA_ALLOC; } AstUnaAlloc;
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_UNA_ALLOC; AstNode n; } AstUna;

#define AST_UNA(kind, token) \
	((AstUnaAlloc){ \
		.node_pos = AST_NODE_WITH_POS(kind, (token).pos), \
	}) \

////////////////////////////////
// bin

#define AST_BIN_ALLOC AstNodeWithPos node_pos; AstIdx lhs
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_BIN_ALLOC; } AstBinAlloc;
typedef struct ALIGNED(AST_NODE_ALIGN) { AST_BIN_ALLOC; AstNode rhs; } AstBin;

#define AST_BIN(kind, token, _lhs) \
	((AstBinAlloc){ \
		.node_pos = AST_NODE_WITH_POS(kind, (token).pos), \
		.lhs      = (_lhs), \
	}) \

////////////////////////////////
// sequence

#define AST_SEQ_ALLOC \
	AstNodeWithSeq node_seq; \
	AstPos open, close;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AST_SEQ_ALLOC;
} AstSeqAlloc;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AST_SEQ_ALLOC;
	AstIdx exprs[];
} AstSeq;

#define AST_SEQ(delim, bound, count, _open, _close) \
	((AstSeqAlloc){ \
		.node_seq = AST_NODE_WITH_SEQ(AST_K_SEQ, delim, bound, count), \
		.open = (_open), .close = (_close), \
	}) \

#define AST_SEQ_DELIM(seq) AST_NODE_SEQ_DELIM(&(seq)->node_seq)
#define AST_SEQ_BOUND(seq) AST_NODE_SEQ_BOUND(&(seq)->node_seq)
#define AST_SEQ_DELIM_ARRAY(seq) ((AstPos*)(void*)&(seq)->exprs[AST_SEQ_COUNT((node)->head_seq)])

////////////////////////////////
// call

#define AST_CALL_ALLOC \
	AstNodeWithSeq node_seq; \
	AstPos open, close; AstIdx fun;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AST_CALL_ALLOC;
} AstCallAlloc;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AST_CALL_ALLOC;
	AstIdx exprs[];
} AstCall;

#define AST_CALL_POSTFIX(_fun, _open, _close, delim, bound, count) \
	((AstCallAlloc){ \
		.node_seq = AST_NODE_WITH_SEQ(AST_K_CALL_POSTFIX, delim, bound, count), \
		.fun = (_fun), .open = (_open), .close = (_close), \
	})

#define AST_CALL_PREFIX(_fun, _open, _close, delim, bound, count) \
	((AstCallAlloc){ \
		.node_seq = AST_NODE_WITH_SEQ(AST_K_CALL_PREFIX, delim, bound, count), \
		.fun = (_fun), .open = (_open), .close = (_close), \
	})


////////////////////////////////
// root

#define AST_ROOT_ALLOC AstNodeWithSeq node_seq;

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AST_ROOT_ALLOC;
} AstRootAlloc;

#define AST_ROOT(delim, count) \
	((AstRootAlloc){ \
		.node_seq = AST_NODE_WITH_SEQ(AST_K_ROOT, delim, AST_B_EOF, count), \
	})

typedef struct ALIGNED(AST_NODE_ALIGN) {
	AST_ROOT_ALLOC;
	AstIdx exprs[];
} AstRoot;

////////////////////////////////
// ast

typedef struct { VIRT_VEC_EMBED(AstIdx); } AstIndexes;

typedef struct {
	VirtStack pool;
	Src src;
} Ast;

void ast_graphviz(FmtVirt * fmt, Ast const * ast, AstIdx idx);
void ast_fmt(FmtVirt * fmt, Ast const * ast, AstIdx idx);

void * ast_idx_to_ptr(Ast const * ast, AstIdx idx);
AstIdx ast_ptr_to_idx(CallLoc call, Ast const * ast, void const * ptr);

#define AST_IDX_TO_PTR(ast, idx) ast_idx_to_ptr(ast, idx)
#define AST_PTR_TO_IDX(ast, ptr) ast_ptr_to_idx(CALL, ast, ptr)

#endif // RK_PRS_AST_H
