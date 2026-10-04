#include "risk/src/ast/set.h"
#include "std/flow/core.h"

AstBound lex_into_bound(LexTag tag) {
	switch (tag) {
		#define X(NAME) case LEX_T_##NAME##_OPEN: case LEX_T_##NAME##_CLOSE: return AST_B_##NAME;
		LEX_PARENS(X)
		#undef X
		default: PANIC("unhandled or invalid [tag]");
	};
};

b8 lex_is_pivot(LexTag tag) {
	switch (tag) {
		#define X(TOKEN) case LEX_T_##TOKEN:
		LEX_PIVOT(X)
		#undef X
			return true;
		default: return false;
	};
};

b8 lex_is_expr(LexTag tag) {
	switch (tag) {
		#define X(TOKEN) case LEX_T_##TOKEN:
		LEX_EXPR(X)
		#undef X
			return true;
		default: return false;
	};
};

