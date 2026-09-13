#include "fmt/lex.h"

#define AT(x) (((u8 const *)lexer->pos)[x])

static bool fmt_l_at_arg_start(FmtLexer * lexer) {
	usz const len = lexer->end - lexer->pos;
	return len > 0 && AT(0) == '{' && (len == 1 || AT(1) != '{');
};

static bool fmt_l_at_arg_end(FmtLexer * lexer) {
	usz const len = lexer->end - lexer->pos;
	return len > 0 && AT(0) == '}' && (len == 1 || AT(1) != '}');
};

static i32 fmt_l_at_escaped(FmtLexer * lexer) {
	usz const len = lexer->end - lexer->pos;
	bool lt = len > 0 && AT(0) == '{' && (len == 1 || AT(1) == '{');
	if (lt) return -1;
	bool rt = len > 0 && AT(0) == '}' && (len == 1 || AT(1) == '}');
	if (rt) return +1;
	return 0;
};

static FmtToken fmt_l_arg(FmtLexer * lexer) {
	uptr start = lexer->pos; lexer->pos += 1;

	for (;;) {
		if (lexer->pos == lexer->end) {
			return (FmtToken){
				.kind = FMT_TOKEN_ARG_UNTERMINED,
				.as.untermined = {.start = (usz)(start - lexer->base)},
			};
		};
		if (fmt_l_at_arg_end(lexer) || *lexer->pos == ':') break;
		lexer->pos += 1;
	}

	bool const optional = start[1] == '?';
	str name = {
		.ptr = start + 1 + optional,
		.len = (usz)(lexer->pos - start - 1 - optional),
	};
	str opt = {.ptr = lexer->pos, .len = 0};

	if (*lexer->pos == ':') {
		lexer->pos += 1;
		uptr opt_start = lexer->pos;
		for (;;) {
			if (lexer->pos == lexer->end) return (FmtToken){
				.kind = FMT_TOKEN_ARG_UNTERMINED,
				.as.untermined = {.start = start - lexer->base},
			};
			if (fmt_l_at_arg_end(lexer)) break;
			lexer->pos += 1;
		};

		opt = (str){
			.ptr = opt_start,
			.len = lexer->pos - opt_start,
		};
	};

	lexer->pos += 1; // `}`
	return (FmtToken){
		.kind = FMT_TOKEN_ARG,
		.as.arg = {.name = name, .opt = opt, .optional = optional},
	};
}

FmtToken fmt_lex_next(FmtLexer * lexer) {
	if (lexer->pos == lexer->end) return (FmtToken){.kind = FMT_TOKEN_EOF};
	
	if (fmt_l_at_arg_start(lexer)) return fmt_l_arg(lexer);

	if (fmt_l_at_arg_end(lexer)) {
		uptr start = lexer->pos;
		lexer->pos += 1;
		return (FmtToken){
			.kind = FMT_TOKEN_NOT_ESCAPED_BRACE,
			.as.not_escaped_brace = start - lexer->base,
		};
	}

	uptr start = lexer->pos;
	i32 escaped = fmt_l_at_escaped(lexer);
	if (escaped != 0) lexer->pos += 2;
	if (escaped < 0) return (FmtToken){.kind = FMT_TOKEN_LT_BRACE};
	if (escaped > 0) return (FmtToken){.kind = FMT_TOKEN_RT_BRACE};

	for (;;) {
		lexer->pos += 1;
		if (
			lexer->pos == lexer->end ||
			fmt_l_at_arg_start(lexer) ||
			fmt_l_at_arg_end(lexer) ||
			fmt_l_at_escaped(lexer) != 0
		) {
			str str = {.ptr = start, .len = lexer->pos - start};
			return (FmtToken){.kind = FMT_TOKEN_STR, .as.str = str};
		}
	}
}
