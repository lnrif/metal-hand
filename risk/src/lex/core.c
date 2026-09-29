#include "risk/src/lex/core.h"

// |================================================================================================|
// |> TOKEN TAG                                                                                    |

Str lex_name(LexTag tag) {
		switch (tag) {
		case LEX_T_EOF: return S("eof");
		case LEX_T_ILLEGAL: return S("illegal");

		case LEX_T_IDENT: return S("ident");
		case LEX_T_NUMBER: return S("number");

		case LEX_T_STRING: return S("string");
		case LEX_T_STRING_UNTERMINATED: return S("string");

		case LEX_T_STRING_LINE: return S("string-line");

		// case LEX_T_STRING_MULTI: return S("multi-string");
		// case LEX_T_STRING_MULTI_UNTERMINATED: return S("multi-string");

		case LEX_T_PLUS_PERCENT_EQ: return S("'+%='");
		case LEX_T_PLUS_EQ: return S("'+='");

		case LEX_T_PLUS_PERCENT: return S("'+%'");

		case LEX_T_PLUS_PLUS: return S("'++'");
		case LEX_T_PLUS: return S("'+'");

		case LEX_T_MINUS_PERCENT_EQ: return S("'-%='");
		case LEX_T_MINUS_EQ: return S("'-='");

		case LEX_T_MINUS_PERCENT: return S("'-%'");

		case LEX_T_MINUS_MINUS_MINUS: return S("'---'");
		case LEX_T_MINUS_GT: return S("'->'");
		case LEX_T_MINUS: return S("'-'");

		case LEX_T_STAR_PERCENT_EQ: return S("'*%='");
		case LEX_T_STAR_EQ: return S("'*='");

		case LEX_T_STAR_PERCENT: return S("'*%'");

		case LEX_T_STAR_STAR: return S("'**'");
		case LEX_T_STAR: return S("'*'");

		case LEX_T_SLASH_EQ: return S("'/='");
		case LEX_T_SLASH: return S("'/'");

		case LEX_T_PERCENT: return S("'%'");

		case LEX_T_AND_AND: return S("'&&'");
		case LEX_T_AND: return S("'&'");

		case LEX_T_OR_OR: return S("'||'");
		case LEX_T_OR: return S("'|'");

		case LEX_T_CARET: return S("'^'");

		case LEX_T_NOT_EQ: return S("'!='");
		case LEX_T_EQ_EQ: return S("'=='");
		case LEX_T_EQ_GT: return S("'=>'");
		case LEX_T_EQ: return S("'='");

		case LEX_T_LT_EQ: return S("'<='");
		case LEX_T_LT_OR: return S("'<|'");
		case LEX_T_LT_TILDE: return S("'<~'");
		case LEX_T_LT_MINUS: return S("'<-'");
		case LEX_T_LT: return S("'<'");

		case LEX_T_GT_EQ: return S("'>='");
		case LEX_T_GT: return S("'>'");

		case LEX_T_TILDE_TILDE: return S("'~~'");
		case LEX_T_TILDE_GT: return S("'~>'");
		case LEX_T_TILDE: return S("'~'");

		case LEX_T_DOT_DOT_EQ: return S("'..='");
		case LEX_T_DOT_DOT_LT: return S("'..<'");
		case LEX_T_DOT_DOT_DOT: return S("'...'");
		case LEX_T_DOT_DOT: return S("'..'");
		case LEX_T_DOT_STAR: return S("'.*'");
		case LEX_T_DOT_AND: return S("'.&'");
		case LEX_T_DOT: return S("'.'");

		case LEX_T_COLON_COLON: return S("'::'");
		case LEX_T_COLON_EQ: return S("':='");
		case LEX_T_COLON: return S("':'");

		case LEX_T_SEMI: return S("';'");
		case LEX_T_COMMA: return S("','");
		case LEX_T_TICK: return S("'''");

		case LEX_T_BANG_BANG: return S("'!!'");
		case LEX_T_BANG: return S("'!'");

		case LEX_T_QMARK_QMARK_QMARK: return S("'\?\?\?'");
		case LEX_T_QMARK_QMARK: return S("'\?\?'");
		case LEX_T_QMARK: return S("'?'");

		case LEX_T_AT: return S("'@'");

		case LEX_T_PAREN_OPEN: return S("'('");
		case LEX_T_PAREN_CLOSE: return S("')'");
		case LEX_T_BRACKET_OPEN: return S("'['");
		case LEX_T_BRACKET_CLOSE: return S("']'");
		case LEX_T_BRACE_OPEN: return S("'{'");
		case LEX_T_BRACE_CLOSE: return S("'}'");

		////////////////////////////////
		// keywords

		case LEX_T_IF: return S("'if'");
		case LEX_T_DO: return S("'do'");
		case LEX_T_ELIF: return S("'elif'");
		case LEX_T_ELSE: return S("'else'");

		case LEX_T_FOR: return S("'for'");
		case LEX_T_LOOP: return S("'loop'");

		case LEX_T_CONTINUE: return S("'continue'");
		case LEX_T_BREAK: return S("'break'");
		case LEX_T_DEFER: return S("'defer'");
		case LEX_T_RET: return S("'ret'");

		////////////////////////////////
		// spaces

		case LEX_T_COMMENT: return S("comment");
		case LEX_T_SPACE: return S("space");
		case LEX_T_NEWLINE: return S("newline");
	};

	return S("UNKNOWN");
};

// |================================================================================================|
// |> LEXER                                                                                         |

Lexer lex_init(StrZ src) {
	return (Lexer){
		.base = src.any, .start = src.any, .at = src.any,
		.end = src.raw + src.len,
	};
};

static b8 is_atom(u8 b) {
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

#define SKIP_AND_FINISH(len, tag) do { AT += len; return finish(lexer, tag); } while (0)
#define FINISH(tag) do { return finish(lexer, tag); } while (0)

#define SKIP_DO_WHILE_AND_FINISH(condition, tag) do { \
	for (;;) {  \
		AT += 1; \
		if (P1 == '\0' || !(condition)) FINISH(tag); \
	} \
} while (0)

#define DO_WHILE_INCLUDED(cond, tag) do { \
	AT += 1; \
	for (;;) { \
		u8 const p = P1; \
		if (p == '\0') break; \
		AT += 1; \
		if (!(cond)) break; \
	}; \
	FINISH(tag); \
} while (0)

#define EAT(b)  (eat(lexer, b))
#define PEEK(b) (P1 == (b))

static inline u8 eat(Lexer * lexer, u8 b) {
	if (P1 != b) return false;
	lexer->at += 1;
	return true;
};

static LexToken finish(Lexer * lexer, LexTag tag) {
	u32 const pos = (u32)(lexer->start - lexer->base);
	u16 const len = (u16)(lexer->at    - lexer->start);
	lexer->start = lexer->at;
	return LEX_TOKEN(pos, len, tag);
};

static b8 is_space  (u8 b) { return b == ' ' || b == '\t' || b == '\r'; };
static b8 is_illegal(u8 b) { return b > 126 || (b < 32 && b != '\t' && b != '\n' && b != '\r'); };

LexToken lex_next(Lexer * lexer) {
	// valid eof
	if (P1 == '\0' && AT == END) FINISH(LEX_T_EOF);

	// unprintable (not ' ', '\t', '\r', '\n') and utf-8
	if (is_illegal(P1)) for (;;) {
		AT += 1;
		// we cannot check `P1 == '\0'`, because `\0` is illegal byte (if not in end).
		// so use `AT == END`
		if (AT == END || !is_illegal(P1)) FINISH(LEX_T_ILLEGAL);
	};

	if EAT('\n')                FINISH(LEX_T_NEWLINE);
	if (is_space(P1))           SKIP_DO_WHILE_AND_FINISH(is_space(P1), LEX_T_SPACE);
	if (P1 == '#')              DO_WHILE_INCLUDED(p != '\n', LEX_T_COMMENT);
	// if (P1 == '/' && P2 == '/') SKIP_DO_WHILE_AND_FINISH(P1 != '\n', LEX_T_TYPO_SLASH_COMMENT);

	if (P1 == '/' && P2 == '/') DO_WHILE_INCLUDED(p != '\n', LEX_T_STRING_LINE);

	// we allow use in some invalid bytes (like hex in decimal).
	// so compiler output less cascadic errors
	//
	// the task of the lexer is to determine token boundaries (that is, what the programmer actually intended),
	// rather than simply splitting the input data into groups.
	if ('0' <= P1 && P1 <= '9')    SKIP_DO_WHILE_AND_FINISH(is_atom(P1), LEX_T_NUMBER);
	if (P1 == '@' && !is_atom(P2)) SKIP_AND_FINISH(1, LEX_T_AT);

	if (
		P1 == 'i' &&
		P2 == 'f' && !is_atom(P3)) SKIP_AND_FINISH(2, LEX_T_IF);

	if (
		P1 == 'd' &&
		P2 == 'o' && !is_atom(P3)) SKIP_AND_FINISH(2, LEX_T_DO);

	if (
		P1 == 'e' &&
		P2 == 'l' &&
		P3 == 'i' &&
		P4 == 'f' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_T_ELIF);

	if (
		P1 == 'e' &&
		P2 == 'l' &&
		P3 == 's' &&
		P4 == 'e' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_T_ELSE);

	if (
		P1 == 'f' &&
		P2 == 'o' &&
		P3 == 'r' && !is_atom(P4)) SKIP_AND_FINISH(3, LEX_T_FOR);

	if (
		P1 == 'l' &&
		P2 == 'o' &&
		P3 == 'o' &&
		P4 == 'p' && !is_atom(P5)) SKIP_AND_FINISH(4, LEX_T_LOOP);

	if (
		P1 == 'c' &&
		P2 == 'o' &&
		P3 == 'n' &&
		P4 == 't' &&
		P5 == 'i' &&
		P6 == 'n' &&
		P7 == 'u' &&
		P8 == 'e' && !is_atom(P9)) SKIP_AND_FINISH(8, LEX_T_CONTINUE);

	if (
		P1 == 'd' &&
		P2 == 'e' &&
		P3 == 'f' &&
		P4 == 'e' &&
		P5 == 'r' && !is_atom(P6)) SKIP_AND_FINISH(5, LEX_T_DEFER);

	if (
		P1 == 'r' &&
		P2 == 'e' &&
		P3 == 't' && !is_atom(P4)) SKIP_AND_FINISH(3, LEX_T_RET);

	if (is_atom(P1)) SKIP_DO_WHILE_AND_FINISH(is_atom(P1), LEX_T_IDENT);

	// one line string
	if (P1 == '"') for (;;) { AT += 1;
		if (P1 == '\\' && P2 == '"') AT += 2;
		if (P1 == '\0' || P1 == '\n') FINISH(LEX_T_STRING_UNTERMINATED);
		if (EAT('"'))                 FINISH(LEX_T_STRING);
	};

	if EAT('+') {
		if EAT('=') FINISH(LEX_T_PLUS_EQ);
		if EAT('%') {
			if EAT('=') FINISH(LEX_T_PLUS_PERCENT_EQ);
			FINISH(LEX_T_PLUS_PERCENT);
		};
		if EAT('+') FINISH(LEX_T_PLUS_PLUS);
		FINISH(LEX_T_PLUS);
	};

	if EAT('-') {
		if EAT('=') FINISH(LEX_T_MINUS_EQ);
		if EAT('>') FINISH(LEX_T_MINUS_GT);
		if EAT('%') {
			if EAT('=') FINISH(LEX_T_MINUS_PERCENT_EQ);
			FINISH(LEX_T_MINUS_PERCENT);
		};
		if (P1 == '-' && P2 == '-') SKIP_AND_FINISH(2, LEX_T_MINUS_MINUS_MINUS);
		FINISH(LEX_T_MINUS);
	};

	if EAT('*') {
		if EAT('=') FINISH(LEX_T_STAR_EQ);
		if EAT('%') {
			if EAT('=') FINISH(LEX_T_STAR_PERCENT_EQ);
			FINISH(LEX_T_STAR_PERCENT);
		};
		if EAT('*') FINISH(LEX_T_STAR_STAR);
		FINISH(LEX_T_STAR);
	};

	if EAT('/') {
		if EAT('=') FINISH(LEX_T_SLASH_EQ);
		FINISH(LEX_T_SLASH);
	};

	if EAT('%') FINISH(LEX_T_PERCENT);

	if EAT('&') {
		if EAT('&') FINISH(LEX_T_AND_AND);
		FINISH(LEX_T_AND);
	};

	if EAT('|') {
		if EAT('|') FINISH(LEX_T_OR_OR);
		FINISH(LEX_T_OR);
	};

	if EAT('^') FINISH(LEX_T_CARET);

	if EAT('=') {
		if EAT('=') FINISH(LEX_T_EQ_EQ);
		if EAT('>') FINISH(LEX_T_EQ_GT);
		FINISH(LEX_T_EQ);
	};

	if EAT('<') {
		if EAT('=') FINISH(LEX_T_LT_EQ);
		if EAT('|') FINISH(LEX_T_LT_OR);
		if EAT('~') FINISH(LEX_T_LT_TILDE);
		if EAT('-') FINISH(LEX_T_LT_MINUS);
		FINISH(LEX_T_LT);
	};

	if EAT('>') {
		if EAT('=') FINISH(LEX_T_GT_EQ);
		// if EAT('|') {
		// 	while (P1 != '\0' && P1 != '\n') AT += 1;
		// 	FINISH(LEX_T_STRING_MULTI);
		// };
		FINISH(LEX_T_GT);
	};

	if EAT('~') {
		if EAT('~') FINISH(LEX_T_TILDE_TILDE);
		if EAT('>') FINISH(LEX_T_TILDE_GT);
		FINISH(LEX_T_TILDE);
	};

	if EAT('.') {
		if EAT('.') {
			if EAT('.') FINISH(LEX_T_DOT_DOT_DOT);
			if EAT('=') FINISH(LEX_T_DOT_DOT_EQ);
			if EAT('<') FINISH(LEX_T_DOT_DOT_LT);
			FINISH(LEX_T_DOT_DOT);
		};
		if EAT('*') FINISH(LEX_T_DOT_STAR);
		if EAT('&') FINISH(LEX_T_DOT_AND);
		FINISH(LEX_T_DOT);
	};

	if EAT(':') {
		if EAT(':') FINISH(LEX_T_COLON_COLON);
		if EAT('=') FINISH(LEX_T_COLON_EQ);
		FINISH(LEX_T_COLON);
	};

	if EAT(';')  FINISH(LEX_T_SEMI);
	if EAT(',')  FINISH(LEX_T_COMMA);
	if EAT('\'') FINISH(LEX_T_TICK);

	if EAT('!') {
		if EAT('=') FINISH(LEX_T_NOT_EQ);
		if EAT('!') FINISH(LEX_T_BANG_BANG);
		FINISH(LEX_T_BANG);
	};

	if EAT('?') {
		if EAT('?') {
			if EAT('?') FINISH(LEX_T_QMARK_QMARK_QMARK);
			FINISH(LEX_T_QMARK_QMARK);
		};
		FINISH(LEX_T_QMARK);
	};

	if EAT('(') FINISH(LEX_T_PAREN_OPEN);
	if EAT(')') FINISH(LEX_T_PAREN_CLOSE);
	if EAT('[') FINISH(LEX_T_BRACKET_OPEN);
	if EAT(']') FINISH(LEX_T_BRACKET_CLOSE);
	if EAT('{') FINISH(LEX_T_BRACE_OPEN);
	if EAT('}') FINISH(LEX_T_BRACE_CLOSE);

	// if EAT('$')  { while (EAT('$'))  {}; FINISH(LEX_T_TYPO_DOLLAR); };
	// if (P1 == '`') for (;;) {
	// 	AT += 1;
	// 	if (P1 == '\\' && P2 == '`')  AT += 2;
	// 	if (P1 == '\0' || P1 == '\n') FINISH(LEX_T_TYPO_QUOTES_UNTERMINATED);
	// 	if (EAT('`'))                 FINISH(LEX_T_TYPO_QUOTES);
	// };

	// if EAT('>') {
	// 	if (EAT('>') || EAT(':')) {
	// 		while (P1 != '\0' && P1 != '\n') AT += 1;
	// 		FINISH(LEX_T_STRING_MULTI);
	// 	};
	//
	// 		while (P1 != '\0' && P1 == '\\') AT += 1;
	// 		FINISH(LEX_T_TYPO_BACK_SLASH);
	// };

	FINISH(LEX_T_ILLEGAL);

	// PANIC("not handled `%c` (%hhu)", P1, P1);
};

