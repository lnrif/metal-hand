#include "lex/lexer.h"
#include "trc/assert.h"
#include "mem/mem.h"

////////////////////////////////////////////////////////////////////////////////
// lexer

Lexer lex_init(StrZ src) {
	usz const limit = 1ULL << 24;
	ASSERT_DEBUG(src.len < limit, "source is too big, limit is %lluMB", limit >> 20);
	ASSERT_DEBUG(src.ptr[src.len] == '\0', "expected zero terminator `\\0` at end of source!");
	return (Lexer){
		.base = src.ptr,
		.start = src.ptr,
		.at = src.ptr,
		.end = src.ptr + src.len,
	};
};

static bool is_atom(u8 b) {
	return ('0' <= b && b <= '9') ||
	       ('A' <= b && b <= 'Z') ||
	       ('a' <= b && b <= 'z') ||
	       b == '_' ||
	       b == '@';
};

#define AT     (lexer->at)
#define END    (lexer->end)

#define P1 (lexer->at[0])
#define P2 (lexer->at[1])
#define P3 (lexer->at[2])
#define P4 (lexer->at[3])
#define P5 (lexer->at[4])
#define P6 (lexer->at[5])
#define P7 (lexer->at[6])
#define P8 (lexer->at[7])
#define P9 (lexer->at[8])

#define SKIP_AND_FINISH(len, kind) do { AT += len; return finish(lexer, kind); } while (0)
#define FINISH(kind) do { return finish(lexer, kind); } while (0)

#define SKIP_DO_WHILE_AND_FINISH(condition, kind) do { \
	for (;;) {  \
		AT += 1; \
		if (P1 == '\0' || !(condition)) FINISH(kind); \
	} \
} while (0)

#define EAT(b)  (eat(lexer, b))
#define PEEK(b) (P1 == (b))

static inline u8 eat(Lexer * lexer, u8 b) {
	if (P1 != b) return FALSE;
	lexer->at += 1;
	return TRUE;
};

static LexToken finish(Lexer * lexer, LexKind kind) {
	u32 const pos = (u32)(lexer->start - lexer->base);
	u32 const len = (u32)(lexer->at    - lexer->start);
	lexer->start = lexer->at;
	return LEX_TOKEN(kind, pos, len);
};

static bool is_space  (u8 b) { return b == ' ' || b == '\t' || b == '\r' || b == '\n'; };
static bool is_illegal(u8 b) { return b > 126 || (b < 32 && b != '\t' && b != '\n' && b != '\r'); };

LexToken lex_next(Lexer * lexer) {
	// valid eof
	if (P1 == '\0' && AT == END) FINISH(LEX_EOF);

	// unprintable (not ' ', '\t', '\r', '\n') and utf-8
	if (is_illegal(P1)) for (;;) {
		AT += 1;
		// we cannot check `P1 == '\0'`, because `\0` is illegal byte (if not in end).
		// so use `AT == END`
		if (AT == END || !is_illegal(P1)) FINISH(LEX_ILLEGAL);
	};

	if (is_space(P1))           SKIP_DO_WHILE_AND_FINISH(is_space(P1), LEX_EXT_SPACE);
	if (P1 == '#')              SKIP_DO_WHILE_AND_FINISH(P1 != '\n', LEX_EXT_COMMENT);
	if (P1 == '/' && P2 == '/') SKIP_DO_WHILE_AND_FINISH(P1 != '\n', LEX_TYPO_SLASH_COMMENT);

	// we allow use in some invalid bytes (like hex in decimal).
	// so compiler output less cascadic errors
	//
	// the task of the lexer is to determine token boundaries (that is, what the programmer actually intended),
	// rather than simply splitting the input data into groups.
	if ('0' <= P1 && P1 <= '9')    SKIP_DO_WHILE_AND_FINISH(is_atom(P1), LEX_NUMBER);
	if (P1 == '@' && !is_atom(P2)) SKIP_AND_FINISH(1, LEX_AT);

	if (
		P1 == 'i' &&
		P2 == 'f' && !is_atom(P3)) SKIP_AND_FINISH(2, LEX_KW_IF);

	if (
		P1 == 't' &&
		P2 == 'h' &&
		P3 == 'e' &&
		P4 == 'n' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_KW_THEN);

	if (
		P1 == 'e' &&
		P2 == 'l' &&
		P3 == 'i' &&
		P4 == 'f' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_KW_ELIF);

	if (
		P1 == 'e' &&
		P2 == 'l' &&
		P3 == 's' &&
		P4 == 'e' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_KW_ELSE);

	if (
		P1 == 'f' &&
		P2 == 'o' &&
		P3 == 'r' && !is_atom(P4)) SKIP_AND_FINISH(3, LEX_KW_FOR);

	if (
		P1 == 'l' &&
		P2 == 'o' &&
		P3 == 'o' &&
		P4 == 'p' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_KW_LOOP);

	if (
		P1 == 'c' &&
		P2 == 'o' &&
		P3 == 'n' &&
		P4 == 't' &&
		P5 == 'i' &&
		P6 == 'n' &&
		P7 == 'u' &&
		P8 == 'e' && !is_atom(P9)) SKIP_AND_FINISH(8, LEX_KW_CONTINUE);

	if (
		P1 == 'd' &&
		P2 == 'e' &&
		P3 == 'f' &&
		P4 == 'e' &&
		P5 == 'r' && !is_atom(P6)) SKIP_AND_FINISH(5, LEX_KW_DEFER);

	if (
		P1 == 'r' &&
		P2 == 'e' &&
		P3 == 't' && !is_atom(P4)) SKIP_AND_FINISH(3, LEX_KW_RET);

	if (is_atom(P1)) SKIP_DO_WHILE_AND_FINISH(is_atom(P1), LEX_IDENT);

	// one line string
	if (P1 == '"') for (;;) { AT += 1;
		if (P1 == '\\' && P2 == '"') AT += 2;
		if (P1 == '\0' || P1 == '\n') FINISH(LEX_STRING_UNTERMINATED);
		if (EAT('"'))                 FINISH(LEX_STRING);
	};

	if EAT('+') {
		if EAT('=') FINISH(LEX_PLUS_EQ);
		if EAT('%') {
			if EAT('=') FINISH(LEX_PLUS_PERCENT_EQ);
			FINISH(LEX_PLUS_PERCENT);
		};
		if EAT('+') FINISH(LEX_PLUS_PLUS);
		FINISH(LEX_PLUS);
	};

	if EAT('-') {
		if EAT('=') FINISH(LEX_MINUS_EQ);
		if EAT('>') FINISH(LEX_MINUS_GT);
		if EAT('%') {
			if EAT('=') FINISH(LEX_MINUS_PERCENT_EQ);
			FINISH(LEX_MINUS_PERCENT);
		};
		if (P1 == '-' && P2 == '-') SKIP_AND_FINISH(2, LEX_MINUS_MINUS_MINUS);
		FINISH(LEX_MINUS);
	};

	if EAT('*') {
		if EAT('=') FINISH(LEX_STAR_EQ);
		if EAT('%') {
			if EAT('=') FINISH(LEX_STAR_PERCENT_EQ);
			FINISH(LEX_STAR_PERCENT);
		};
		if EAT('*') FINISH(LEX_STAR_STAR);
		FINISH(LEX_STAR);
	};

	if EAT('/') {
		if EAT('=') FINISH(LEX_SLASH_EQ);
		FINISH(LEX_SLASH);
	};

	if EAT('%') FINISH(LEX_PERCENT);

	if EAT('&') {
		if EAT('&') FINISH(LEX_AND_AND);
		FINISH(LEX_AND);
	};

	if EAT('|') {
		if EAT('|') FINISH(LEX_OR_OR);
		FINISH(LEX_OR);
	};

	if EAT('^') FINISH(LEX_CARET);

	if EAT('=') {
		if EAT('=') FINISH(LEX_EQ_EQ);
		if EAT('>') FINISH(LEX_EQ_GT);
		FINISH(LEX_EQ);
	};

	if EAT('<') {
		if EAT('=') FINISH(LEX_LT_EQ);
		if EAT('|') FINISH(LEX_LT_OR);
		if EAT('~') FINISH(LEX_LT_TILDE);
		if EAT('-') FINISH(LEX_LT_MINUS);
		FINISH(LEX_LT);
	};

	if EAT('>') {
		if EAT('=') FINISH(LEX_GT_EQ);
		FINISH(LEX_GT);
	};

	if EAT('~') {
		if EAT('~') FINISH(LEX_TILDE_TILDE);
		if EAT('>') FINISH(LEX_TILDE_GT);
		FINISH(LEX_TILDE);
	};

	if EAT('.') {
		if EAT('.') {
			if EAT('.') FINISH(LEX_DOT_DOT_DOT);
			if EAT('=') FINISH(LEX_DOT_DOT_EQ);
			if EAT('<') FINISH(LEX_DOT_DOT_LT);
			FINISH(LEX_DOT_DOT);
		};
		if EAT('*') FINISH(LEX_DOT_STAR);
		FINISH(LEX_DOT);
	};

	if EAT(':') {
		if EAT(':') FINISH(LEX_COLON_COLON);
		if EAT('=') FINISH(LEX_COLON_EQ);
		FINISH(LEX_COLON);
	};

	if EAT(';')  FINISH(LEX_SEMI);
	if EAT(',')  FINISH(LEX_COMMA);
	if EAT('\'') FINISH(LEX_TICK);

	if EAT('!') {
		if EAT('=') FINISH(LEX_NOT_EQ);
		if EAT('!') FINISH(LEX_BANG_BANG);
		FINISH(LEX_BANG);
	};

	if EAT('?') {
		if EAT('?') {
			if EAT('?') FINISH(LEX_QMARK_QMARK_QMARK);
			FINISH(LEX_QMARK_QMARK);
		};
		FINISH(LEX_QMARK);
	};

	if EAT('(') FINISH(LEX_PAREN_OPEN);
	if EAT(')') FINISH(LEX_PAREN_CLOSE);
	if EAT('[') FINISH(LEX_BRACKET_OPEN);
	if EAT(']') FINISH(LEX_BRACKET_CLOSE);
	if EAT('{') FINISH(LEX_BRACE_OPEN);
	if EAT('}') FINISH(LEX_BRACE_CLOSE);

	if EAT('$')  { while (EAT('$'))  {}; FINISH(LEX_TYPO_DOLLAR); };
	if (P1 == '`') for (;;) {
		AT += 1;
		if (P1 == '\\' && P2 == '`')  AT += 2;
		if (P1 == '\0' || P1 == '\n') FINISH(LEX_TYPO_QUOTES_UNTERMINATED);
		if (EAT('`'))                 FINISH(LEX_TYPO_QUOTES);
	};

	if EAT('\\') {
		if (EAT('>') || EAT(':')) {
			while (P1 != '\0' && P1 != '\n') AT += 1;
			FINISH(LEX_STRING_LINE);
		};

		while (P1 != '\0' && P1 == '\\') AT += 1;
		FINISH(LEX_TYPO_BACK_SLASH);
	};

	PANIC("not handled `%c` (%hhu)", P1, P1);
};

