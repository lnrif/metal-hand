#ifndef RK_LEX_CORE_H
#define RK_LEX_CORE_H

#include "std/str/core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// token kind

// S - simple
// E - extenstion

// X(kind, upper, lower, lit)
#define LEX_KEYWORDS(X) \
	X(LEX_KW_IF, IF, if, "if") \
	X(LEX_KW_THEN, THEN, then, "then") \
	X(LEX_KW_ELIF, ELIF, elif, "elif") \
	X(LEX_KW_ELSE, ELSE, else, "else") \
	\
	X(LEX_KW_FOR, FOR, for, "for") \
	X(LEX_KW_LOOP, LOOP, loop, "loop") \
	\
	X(LEX_KW_CONTINUE, CONTINUE, continue, "continue") \
	X(LEX_KW_DEFER, DEFER, defer, "defer") \
	X(LEX_KW_RET, RET, ret, "ret") \

enum: u8 {
	// end of file
	LEX_EOF,

	// unprintable control codes:
	//  - 0-31 (except for 9 '\t', 10 '\n', 13 '\r')
	// extended ASCII codes or utf-8:
	//  - 127-255
	LEX_ILLEGAL,

	// 'ident_123', '_unused'
	// '@mod', '@load'
	LEX_IDENT,
	// '123_456', '1_000_000', '0Ascii007'
	LEX_NUMBER,

	// "text\n"
	LEX_STRING,
	// '\> text' or '\: text'
	// LEX_STRING_LINE,
	LEX_STRING_UNTERMINATED,

	// not have new line at the end of lines (should write yourself)
	// strips all spaces at start of every line
	// if needed custom offset use '|=|'
	// if needed auto newline write '|>|'
	// use tabs only from '\t', otherwise error
	// ```
	// x := """
	//      |>|#include <stdio.h>
	//      |>|
	//      |>|void main(void) {
	//      |>|    printf("Hello, World\\n");
	//      |>|};
	//      """;
	// ```
	LEX_STRING_MULTI, // TODO

	// """
	// \> any bytes, \e\t\r\n, maybe ends with newline and without \"""
	// """
	LEX_STRING_MULTI_UNTERMINATED,


	// '+%='
	LEX_PLUS_PERCENT_EQ,
	// '+='
	LEX_PLUS_EQ,

	// '+%'
	LEX_PLUS_PERCENT,

	// '++'
	LEX_PLUS_PLUS,
	// '+'
	LEX_PLUS,

	// '-%='
	LEX_MINUS_PERCENT_EQ,
	// '-='
	LEX_MINUS_EQ,

	// '-%'
	LEX_MINUS_PERCENT,

	// '---'
	LEX_MINUS_MINUS_MINUS,
	// '->'
	LEX_MINUS_GT,
	// '-'
	LEX_MINUS,

	// '*%='
	LEX_STAR_PERCENT_EQ,
	// '*='
	LEX_STAR_EQ,

	// '*%'
	LEX_STAR_PERCENT,

	// '**'
	LEX_STAR_STAR,
	// '*'
	LEX_STAR,

	// '/='
	LEX_SLASH_EQ,
	// '/'
	LEX_SLASH,

	// '%'
	LEX_PERCENT,

	// '&&'
	LEX_AND_AND,
	// '&'
	LEX_AND,

	// '||'
	LEX_OR_OR,
	// '|'
	LEX_OR,

	// '^'
	LEX_CARET,

	// '!='
	LEX_NOT_EQ,
	// '=='
	LEX_EQ_EQ,
	// '=>'
	LEX_EQ_GT,
	// '='
	LEX_EQ,

	// '<='
	LEX_LT_EQ,
	// '<|'
	LEX_LT_OR,
	// '<~'
	LEX_LT_TILDE,
	// '<-'
	LEX_LT_MINUS,
	// '<'
	LEX_LT,

	// '>='
	LEX_GT_EQ,
	// '>'
	LEX_GT,

	// '~~'
	LEX_TILDE_TILDE,
	// '~>'
	LEX_TILDE_GT,
	// '~'
	LEX_TILDE,

	// '..='
	LEX_DOT_DOT_EQ,
	// '..<'
	LEX_DOT_DOT_LT,
	// '...'
	LEX_DOT_DOT_DOT,
	// '..'
	LEX_DOT_DOT,
	// '.*'
	LEX_DOT_STAR,
	// '.&'
	// LEX_DOT_AND,
	// '.'
	LEX_DOT,

	// '::'
	LEX_COLON_COLON,
	// ':='
	LEX_COLON_EQ,
	// ':'
	LEX_COLON,

	// ';'
	LEX_SEMI,
	// ','
	LEX_COMMA,
	// '''
	LEX_TICK,

	// '!!'
	LEX_BANG_BANG,
	// '!'
	LEX_BANG,

	// '???'
	LEX_QMARK_QMARK_QMARK,
	// '??'
	LEX_QMARK_QMARK,
	// '?'
	LEX_QMARK,

	// '@'
	LEX_AT,

	// '('
	LEX_PAREN_OPEN,
	// ')'
	LEX_PAREN_CLOSE,
	// '['
	LEX_BRACKET_OPEN,
	// ']'
	LEX_BRACKET_CLOSE,
	// '{'
	LEX_BRACE_OPEN,
	// '}'
	LEX_BRACE_CLOSE,

	////////////////////////////////
	// keywords

	LEX_KW_BEG,
		#define X(kind, _2, _3, _4) kind,
		LEX_KEYWORDS(X)
		#undef X
	LEX_KW_END = LEX_KW_RET,

	// '# any bytes, ends with newline'
	LEX_COMMENT,
	// ' ', '\t', '\r',
	LEX_SPACE,
	// '\n'
	LEX_NEWLINE,

	////////////////////////////////
	// kind count

	LEX_KIND_LAST,
	LEX_KIND_COUNT = LEX_KIND_LAST,

	////////////////////////////////
	// + ext kind count

};

// #define LEX_IS_TYPO(kind) (LEX_EXT_KIND_LAST <= (kind) && (kind) < LEX_TYPO_KIND_LAST)

typedef u8 LexKind;
// typedef u8 LexKindExt;
// typedef u8 LexKindTypo;

Str lex_token_name(LexKind kind);

// |================================================================================================|
// |> TOKEN                                                                                         |
// |================================================================================================|

typedef struct ALIGNED(8) {
	union {
		struct { LexKind kind; u8 _len[3]; };
		u32 kind_and_len;
	};
	u32 pos;
} LexToken;

#define LEX_TOKEN(_kind, _pos, _len) \
	((LexToken){.pos = (_pos), .kind_and_len = (_kind) | ((_len) << 8)})

u32 lex_len(LexToken token);

// |================================================================================================|
// |> LEXER                                                                                         |
// |================================================================================================|

// The lexer's task is to extract tokens—ideally without
// creating cascading tokens that lead to cascading errors.
typedef struct {
	u8 const * base;
	u8 const * start;
	u8 const * at;
	u8 const * end;
} Lexer;

#define LEXER(src) \
	((Lexer){ \
		.base = (src).ptr, \
		.start = (src).ptr, \
		.at = (src).ptr, \
		.end = (src).ptr + (src).len, \
	})

Lexer    lex_init(StrZ src);
LexToken lex_next(Lexer * lexer);

#endif // !RK_LEX_CORE_H
