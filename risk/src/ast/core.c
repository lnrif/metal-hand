#include "risk/src/ast/core.h"
#include "std/mem/bump.h"
#include "std/mem/page.h"

// |================================================================================================|
// |> AST TAG                                                                                       |

Str ast_name(AstTag tag) {
	switch (tag) {

		// case AST_T_ATOM_BEG:
			case AST_T_POISON: return S("poison");
			case AST_T_ATOM_IDENT: return S("ident");
			case AST_T_ATOM_NUMBER: return S("100");
			case AST_T_ATOM_STRING: return S("text");
		// case AST_T_ATOM_END:

		// case AST_T_UNA_BEG:
			case AST_T_UNA_POS: return S("+a");
			case AST_T_UNA_NEG: return S("-a");
			case AST_T_UNA_NOT: return S("!a");
			case AST_T_UNA_DOT: return S(".a");
			case AST_T_UNA_DEREF: return S("a.*");
			case AST_T_UNA_REF: return S("a.&");
		// case AST_T_UNA_END:

		// case AST_T_BIN_BEG:
			case AST_T_BIN_ADD: return S("a + b");
			case AST_T_BIN_SUB: return S("a - b");
			case AST_T_BIN_MUL: return S("a * b");
			case AST_T_BIN_DIV: return S("a / b");

			case AST_T_BIN_ADD_SET: return S("a += b");
			case AST_T_BIN_SUB_SET: return S("a -= b");
			case AST_T_BIN_MUL_SET: return S("a *= b");
			case AST_T_BIN_DIV_SET: return S("a /= b");

			case AST_T_BIN_LT: return S("a < b");
			case AST_T_BIN_LE: return S("a <= b");
			case AST_T_BIN_GT: return S("a > b");
			case AST_T_BIN_GE: return S("a >= b");
			case AST_T_BIN_EQ: return S("a == b");
			case AST_T_BIN_NEQ: return S("a != b");

			case AST_T_BIN_OR: return S("a | b");
			case AST_T_BIN_AND: return S("a & b");

			case AST_T_BIN_LOG_OR: return S("a || b");
			case AST_T_BIN_LOG_AND: return S("a && b");

			case AST_T_BIN_DOT: return S("a.b");
			case AST_T_BIN_TICK: return S("a'b");

			case AST_T_BIN_COLON: return S("a: b");
			case AST_T_BIN_SET: return S("a = b");
			case AST_T_BIN_DEF: return S("a := b");

			case AST_T_BIN_IMPLIES: return S("a => b");
			case AST_T_BIN_ARROW: return S("a -> b");
			case AST_T_BIN_EXTEND: return S("a :: b");
			case AST_T_BIN_APPLY: return S("a <| b");
		// case AST_T_BIN_END:
	
		case AST_T_IF_ELSE: return S("if a then b else c");
		case AST_T_SEQ: return S("a, b, ...");
		case AST_T_CALL: return S("f(a, b, ...)");
	};

	return S("UNKNOWN");
};

// |================================================================================================|
// |> AST POOL                                                                                      |

Ast ast_init(Pages * pages, Src src) {
	return (Ast){.src = src, .bump = bump_init(pages), .root = AST_IDX_NIL};
};

void ast_free(Ast * ast) {
	// TODO: free
};

void * ast_node(Ast * ast, u64 size) {
	return bump_raw(&ast->bump, size, 4).any;
};

