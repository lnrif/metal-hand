#ifndef RK_PRS_PARSER_H
#define RK_PRS_PARSER_H

#include "mem/virt/page.h"
#include "mem/virt/vec.h"
#include "prs/ast.h"
#include "lex/stream.h"

typedef struct {
	VIRT_VEC_EMBED(AstBound);
} PrsOpenVec;

typedef struct {
	FmtVirt * out; Ast raw;
	LexTokenVec tokens; usz at;
	PrsOpenVec opens; VirtBump tmp;
} Parser;

Parser parser_init(Memory * mem, FmtVirt * out, Lex lex);
AstIdx parser_root(Parser * parser);

#endif // !RK_PRS_PARSER_H
