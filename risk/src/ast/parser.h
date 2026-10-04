#ifndef RK_AST_PARSER_H
#define RK_AST_PARSER_H

#include "risk/src/src/core.h"
#include "risk/src/ast/core.h"
#include "risk/src/lex/core.h"

typedef struct {
	Fmt * fmt; Bump tmp;
	Ast ast; Lexer lexer;
	LexToken previous, peek;
	struct { u32 count; u32 limit; } error;
} Parser;

Parser parser_init(Src src, Fmt * fmt, Pages * nodes, Pages * tmp, u32 errors_limit);
Ast parser_parse(Parser * parser);

#endif // !RK_AST_PARSER_H
