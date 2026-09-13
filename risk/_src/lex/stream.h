#ifndef RK_LEX_STREAM_H
#define RK_LEX_STREAM_H

#include "lex/lexer.h"
#include "fmt/fmt.h"
#include "src/src.h"

typedef struct { VIRT_VEC_EMBED(LexToken); } LexTokenVec;

// ENUM(LexDelimKind, u8) {
// 	LEX_DELIM_SEMI,
// 	LEX_DELIM_COMMA,
// 	LEX_DELIM_PAREN,
// 	LEX_DELIM_BRACE,
// };
//
// typedef union {
// 	struct { kind };
// 	u32 load;
// } LexDelim;
//
// #define lex_kind();
//
// typedef struct { VIRT_VEC_EMBED(LexDelim); } LexDelimVec;

typedef struct {
	Src src;
	LexTokenVec tokens;
} Lex;

Lex lex_analyze(Memory * mem, FmtVirt * out, Src src);

#endif // !RK_LEX_STREAM_H

