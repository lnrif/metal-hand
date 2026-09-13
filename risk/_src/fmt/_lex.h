#ifndef RK_FMT_LEX_H
#define RK_FMT_LEX_H

#include "core.h"
#include "str.h"

////////////////////////////////////////////////////////////////////////////////
// format lexer

////////////////////////////////
// token kind

ENUM(FmtTokenKind, u8) {
	FMT_TOKEN_EOF,
	// any bytes
	FMT_TOKEN_STR,
	// `{...}`
	FMT_TOKEN_ARG,
	// `{{` -> `{`
	FMT_TOKEN_LT_BRACE,
	// `}}` -> `}`
	FMT_TOKEN_RT_BRACE,
	// `}`
	FMT_TOKEN_NOT_ESCAPED_BRACE,
	// `{...`
	FMT_TOKEN_ARG_UNTERMINED,
	FMT_TOKEN_COUNT,
};

////////////////////////////////
// token load

typedef struct {
	str name;
	str opt;
	bool optional;
} FmtTokenArg;

////////////////////////////////
// token

typedef struct {
	union {
		str str;
		FmtTokenArg arg;
		usz not_escaped_brace;
		struct { usz start; } untermined;
	} as;
	FmtTokenKind kind;
} FmtToken;

typedef struct {
	uptr beg;
	uptr pos;
	uptr end;
} FmtLexer;

FmtToken fmt_lex_next(FmtLexer * l);

#endif // !FMT_LEX_H
