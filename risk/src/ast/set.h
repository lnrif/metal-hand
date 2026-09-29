#ifndef RK_AST_SET_H
#define RK_AST_SET_H

#include "risk/src/lex/core.h"
#include "risk/src/ast/core.h"

// |================================================================================================|
// |> PARSER SETS                                                                                   |

// #define X(TOKEN, NODE)
#define LEX_AST_INFIX(X) \
	X(PLUS, ADD) \
	X(MINUS, SUB) \
	X(STAR, MUL) \
	X(SLASH, DIV) \
	X(COLON, COLON) \
	X(EQ, SET) \
	X(COLON_EQ, DEF) \
	X(EQ_GT, IMPLIES) \
	X(MINUS_GT, ARROW) \
	X(COLON_COLON, EXTEND) \
	X(LT_OR, APPLY) \
	X(DOT, DOT) \
	X(TICK, TICK) \
	\
	X(LT, LT) \
	X(LT_EQ, LE) \
	X(GT, GT) \
	X(GT_EQ, GE) \
	\
	X(EQ_EQ, EQ) \
	X(NOT_EQ, NEQ) \
	\
	X(AND_AND, LOG_AND) \
	X(OR_OR, LOG_OR) \

// #define X(TOKEN)
#define LEX_PIVOT(X) \
	X(IDENT) \
	X(NUMBER) \
	X(STRING) \
	\
	X(PLUS) \
	X(MINUS) \
	X(BANG) \
	X(DOT) \
	\
	X(PAREN_OPEN) \
	X(BRACE_OPEN) \
	X(BRACKET_OPEN) \
	X(PAREN_CLOSE) \
	X(BRACE_CLOSE) \
	X(BRACKET_CLOSE) \
	\
	X(IF) \

// #define X(NAME)
#define LEX_PARENS(X) \
	X(PAREN) \
	X(BRACE) \
	X(BRACKET) \

AstBound lex_into_bound(LexTag tag);
b8 lex_is_pivot(LexTag tag);

#endif // !RK_AST_SET_H
