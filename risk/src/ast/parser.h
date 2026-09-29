#ifndef RK_AST_PARSER_H
#define RK_AST_PARSER_H

#include "risk/src/src/core.h"
#include "risk/src/ast/core.h"
#include "risk/src/lex/core.h"

typedef struct {
	Fmt * fmt;
	Ast ast; Lexer lexer;
	LexToken peek;
} Parser;

Parser parser_init(Src src, Fmt * fmt, Pages * ast);
Ast parser_parse(Parser * parser);

#endif // !RK_AST_PARSER_H
