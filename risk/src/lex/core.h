#ifndef RK_LEX_CORE_H
#define RK_LEX_CORE_H

#include "std/str/core.h"

// |================================================================================================|
// |> TOKEN TAG                                                                                    |

// X(tag, upper, lower)
#define LEX_KEYWORDS(X) \
	X(LEX_T_IF, IF, if) \
	X(LEX_T_DO, DO, do) \
	X(LEX_T_ELIF, ELIF, elif) \
	X(LEX_T_ELSE, ELSE, else) \
	\
	X(LEX_T_FOR, FOR, for) \
	X(LEX_T_LOOP, LOOP, loop) \
	\
	X(LEX_T_CONTINUE, CONTINUE, continue) \
	X(LEX_T_BREAK, BREAK, break) \
	X(LEX_T_DEFER, DEFER, defer) \
	X(LEX_T_RET, RET, ret) \

typedef enum: u8 {
	// end of file
	LEX_T_EOF,

	// unprintable control codes:
	//  - 0-31 (except for 9 '\t', 10 '\n', 13 '\r')
	// extended ASCII codes or utf-8:
	//  - 127-255
	LEX_T_ILLEGAL,

	// 'ident_123', '_unused'
	// '@mod', '@load'
	LEX_T_IDENT,
	// '123_456', '1_000_000', '0Ascii007'
	LEX_T_NUMBER,

	// "text\n"
	LEX_T_STRING,
	// '\> text' or '\: text'
	// LEX_T_STRING_LINE,
	LEX_T_STRING_UNTERMINATED,

	// // any bytes not escaped
	LEX_T_STRING_LINE,

	// TODO:
	// not have new line at the end of lines (should write yourself)
	// strips all spaces at start of every line
	// if needed custom offset use '|=|'
	// if needed auto newline write '|>|'
	// use tabs only from '\t', otherwise error
	// ```
	// x =>
	//    //#include <stdio.h>
	//    //
	//    //void main(void) {
	//    //    printf("Hello, World\\n");
	//    //};
	// ;
	// ```
	// LEX_T_STRING_MULIT,

	// TODO:
	// """
	// \> any bytes, \e\t\r\n, maybe ends with newline and without \"""
	// """
	// LEX_T_STRING_MULTI_UNTERMINATED,

	// '+%='
	LEX_T_PLUS_PERCENT_EQ,
	// '+='
	LEX_T_PLUS_EQ,

	// '+%'
	LEX_T_PLUS_PERCENT,

	// '++'
	LEX_T_PLUS_PLUS,
	// '+'
	LEX_T_PLUS,

	// '-%='
	LEX_T_MINUS_PERCENT_EQ,
	// '-='
	LEX_T_MINUS_EQ,

	// '-%'
	LEX_T_MINUS_PERCENT,

	// '---'
	LEX_T_MINUS_MINUS_MINUS,
	// '->'
	LEX_T_MINUS_GT,
	// '-'
	LEX_T_MINUS,

	// '*%='
	LEX_T_STAR_PERCENT_EQ,
	// '*='
	LEX_T_STAR_EQ,

	// '*%'
	LEX_T_STAR_PERCENT,

	// '**'
	LEX_T_STAR_STAR,
	// '*'
	LEX_T_STAR,

	// '/='
	LEX_T_SLASH_EQ,
	// '/'
	LEX_T_SLASH,

	// '%'
	LEX_T_PERCENT,

	// '&&'
	LEX_T_AND_AND,
	// '&'
	LEX_T_AND,

	// '||'
	LEX_T_OR_OR,
	// '|'
	LEX_T_OR,

	// '^'
	LEX_T_CARET,

	// '!='
	LEX_T_NOT_EQ,
	// '=='
	LEX_T_EQ_EQ,
	// '=>'
	LEX_T_EQ_GT,
	// '='
	LEX_T_EQ,

	// '<='
	LEX_T_LT_EQ,
	// '<|'
	LEX_T_LT_OR,
	// '<~'
	LEX_T_LT_TILDE,
	// '<-'
	LEX_T_LT_MINUS,
	// '<'
	LEX_T_LT,

	// '>='
	LEX_T_GT_EQ,
	// '>'
	LEX_T_GT,

	// '~~'
	LEX_T_TILDE_TILDE,
	// '~>'
	LEX_T_TILDE_GT,
	// '~'
	LEX_T_TILDE,

	// '..='
	LEX_T_DOT_DOT_EQ,
	// '..<'
	LEX_T_DOT_DOT_LT,
	// '...'
	LEX_T_DOT_DOT_DOT,
	// '..'
	LEX_T_DOT_DOT,
	// '.*'
	LEX_T_DOT_STAR,
	// '.&'
	LEX_T_DOT_AND,
	// '.'
	LEX_T_DOT,

	// '::'
	LEX_T_COLON_COLON,
	// ':='
	LEX_T_COLON_EQ,
	// ':'
	LEX_T_COLON,

	// ';'
	LEX_T_SEMI,
	// ','
	LEX_T_COMMA,
	// '''
	LEX_T_TICK,

	// '!!'
	LEX_T_BANG_BANG,
	// '!'
	LEX_T_BANG,

	// '???'
	LEX_T_QMARK_QMARK_QMARK,
	// '??'
	LEX_T_QMARK_QMARK,
	// '?'
	LEX_T_QMARK,

	// '@'
	LEX_T_AT,

	// '('
	LEX_T_PAREN_OPEN,
	// ')'
	LEX_T_PAREN_CLOSE,
	// '['
	LEX_T_BRACKET_OPEN,
	// ']'
	LEX_T_BRACKET_CLOSE,
	// '{'
	LEX_T_BRACE_OPEN,
	// '}'
	LEX_T_BRACE_CLOSE,

	////////////////////////////////
	// keywords

	#define X(tag, upper, lower) tag,
	LEX_KEYWORDS(X)
	#undef X

	////////////////////////////////
	// spaces

	// '# any bytes, ends with newline'
	LEX_T_COMMENT,
	// ' ', '\t', '\r',
	LEX_T_SPACE,
	// '\n'
	LEX_T_NEWLINE,
} LexTag;

#define LEX_IS_SPACES(tag) ((tag) >= LEX_T_COMMENT)

// #define LEX_T_BEG LEX_T_IF
// #define LEX_T_END (LEX_T_RET + 1)

#define LEX_TAG_COUNT ((u8)LEX_T_NEWLINE + 1)

Str lex_name(LexTag tag);

// |================================================================================================|
// |> TOKEN                                                                                         |
// |================================================================================================|

typedef struct ALIGNED(8) {
	u32 pos;
	u16 len;
	LexTag tag;
} LexToken;

#define LEX_TOKEN(_pos, _len, _tag) \
	((LexToken){.pos = (_pos), .len = (_len), .tag = (_tag)})

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

// #define LEXER(src) \
// 	((Lexer){ \
// 		.base = (src).ptr, \
// 		.start = (src).ptr, \
// 		.at = (src).ptr, \
// 		.end = (src).ptr + (src).len, \
// 	})

Lexer    lex_init(StrZ src);
LexToken lex_next(Lexer * lexer);

#endif // !RK_LEX_CORE_H
