#ifndef RK_LEXER_H
#define RK_LEXER_H

#include "core/macro.h"
#include "mem/mem.h"
#include "str/str.h"

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
	LEX_EOF, // DONE

	// unprintable control codes:
	//  - 0-31 (except for 9 '\t', 10 '\n', 13 '\r')
	// extended ASCII codes or utf-8:
	//  - 127-255
	LEX_ILLEGAL, // DONE

	// 'ident_123', '_unused'
	// '@mod', '@load'
	LEX_IDENT, // DONE
	// '123_456', '1_000_000', '0Ascii007'
	LEX_NUMBER, // DONE

	// "text\n"
	LEX_STRING, // DONE
	// '\> text' or '\: text'
	LEX_STRING_LINE, // DONE
	LEX_STRING_UNTERMINATED, // DONE

	// // """
	// // \| any bytes, \e\t\r\n, maybe ends with newline and without \"""
	// LEX_UNTERMINATED_MULTILINE, // TODO

	// '+%='
	LEX_PLUS_PERCENT_EQ, // DONE
	// '+='
	LEX_PLUS_EQ, // DONE

	// '+%'
	LEX_PLUS_PERCENT, // DONE

	// '++'
	LEX_PLUS_PLUS, // DONE
	// '+'
	LEX_PLUS, // DONE

	// '-%='
	LEX_MINUS_PERCENT_EQ, // DONE
	// '-='
	LEX_MINUS_EQ, // DONE

	// '-%'
	LEX_MINUS_PERCENT, // DONE

	// '---'
	LEX_MINUS_MINUS_MINUS, // DONE
	// '->'
	LEX_MINUS_GT, // DONE
	// '-'
	LEX_MINUS, // DONE

	// '*%='
	LEX_STAR_PERCENT_EQ, // DONE
	// '*='
	LEX_STAR_EQ, // DONE

	// '*%'
	LEX_STAR_PERCENT, // DONE

	// '**'
	LEX_STAR_STAR, // DONE
	// '*'
	LEX_STAR, // DONE

	// '/='
	LEX_SLASH_EQ, // DONE
	// '/'
	LEX_SLASH, // DONE

	// '%'
	LEX_PERCENT, // DONE

	// '&&'
	LEX_AND_AND, // DONE
	// '&'
	LEX_AND, // DONE

	// '||'
	LEX_OR_OR, // DONE
	// '|'
	LEX_OR, // DONE

	// '^'
	LEX_CARET, // DONE

	// '!='
	LEX_NOT_EQ, // DONE
	// '=='
	LEX_EQ_EQ, // DONE
	// '=>'
	LEX_EQ_GT, // DONE
	// '='
	LEX_EQ, // DONE

	// '<='
	LEX_LT_EQ, // DONE
	// '<|'
	LEX_LT_OR, // DONE
	// '<~'
	LEX_LT_TILDE, // DONE
	// '<-'
	LEX_LT_MINUS, // DONE
	// '<'
	LEX_LT, // DONE

	// '>='
	LEX_GT_EQ, // DONE
	// '>'
	LEX_GT, // DONE

	// '~~'
	LEX_TILDE_TILDE, // DONE
	// '~>'
	LEX_TILDE_GT, // DONE
	// '~'
	LEX_TILDE, // DONE

	// '..='
	LEX_DOT_DOT_EQ, // DONE
	// '..<'
	LEX_DOT_DOT_LT, // DONE
	// '...'
	LEX_DOT_DOT_DOT, // DONE
	// '..'
	LEX_DOT_DOT, // DONE
	// '.*'
	LEX_DOT_STAR, // DONE
	// '.'
	LEX_DOT, // DONE

	// '::'
	LEX_COLON_COLON, // DONE
	// ':='
	LEX_COLON_EQ, // DONE
	// ':'
	LEX_COLON, // DONE

	// ';'
	LEX_SEMI, // DONE
	// ','
	LEX_COMMA, // DONE
	// '''
	LEX_TICK, // DONE

	// '!!'
	LEX_BANG_BANG, // DONE
	// '!'
	LEX_BANG, // DONE

	// '???'
	LEX_QMARK_QMARK_QMARK, // DONE
	// '??'
	LEX_QMARK_QMARK, // DONE
	// '?'
	LEX_QMARK, // DONE

	// '@'
	LEX_AT, // DONE

	// '('
	LEX_PAREN_OPEN, // DONE
	// ')'
	LEX_PAREN_CLOSE, // DONE
	// '['
	LEX_BRACKET_OPEN, // DONE
	// ']'
	LEX_BRACKET_CLOSE, // DONE
	// '{'
	LEX_BRACE_OPEN, // DONE
	// '}'
	LEX_BRACE_CLOSE, // DONE

	////////////////////////////////
	// keywords

	LEX_KW_BEG,
		#define X(kind, _2, _3, _4) kind,
		LEX_KEYWORDS(X)
		#undef X
	LEX_KW_END = LEX_KW_RET,

	////////////////////////////////
	// kind count

	LEX_KIND_LAST,
	LEX_KIND_COUNT = LEX_KIND_LAST,

	////////////////////////////////
	// extended kinds

	// '# any bytes, ends with newline'
	LEX_EXT_COMMENT = LEX_KIND_LAST, // DONE
	// ' ', '\t', '\r', '\n'
	LEX_EXT_SPACE, // DONE

	// not have new line at the end of lines (should write yourself)
	// strips all spaces at start of every line
	// if needed custom offset use '\|'
	// if needed auto newline write '\>'
	// use tabs only from '\t'
	//
	// x := """
	//      \> #include <stdio.h>
	//      """
	//      ++ "\n" ++
	//      """
	//      \> void main(void) {
	//      \>     printf("Hello, World\\n");
	//      \> };
	//      """;
	// LEX_EXT_STRING_MULTILINE, // TODO

	////////////////////////////////
	// + ext kind count

	LEX_EXT_KIND_LAST,
	LEX_EXT_KIND_COUNT = LEX_EXT_KIND_LAST,

	////////////////////////////////
	// typo kinds

	// '// any bytes, ends with newline'
	LEX_TYPO_SLASH_COMMENT = LEX_EXT_KIND_LAST, // DONE

	// '`text`'
	LEX_TYPO_QUOTES, // DONE
	// '`text'
	LEX_TYPO_QUOTES_UNTERMINATED, // DONE

	// '$...'
	LEX_TYPO_DOLLAR, // DONE
	// '\...'
	LEX_TYPO_BACK_SLASH, // DONE

	////////////////////////////////
	// + typo kind count

	LEX_TYPO_KIND_LAST,
	LEX_TYPO_KIND_COUNT = LEX_TYPO_KIND_LAST,
};

#define LEX_IS_TYPO(kind) \
	(LEX_EXT_KIND_LAST <= (kind) && (kind) < LEX_TYPO_KIND_LAST)

typedef u8 LexKind;
typedef u8 LexKindExt;
typedef u8 LexKindTypo;

Str lex_token_name(LexKindTypo kind);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// token

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

////////////////////////////////////////////////////////////////////////////////
// lexer

// The lexer's task is to extract tokens—ideally without creating cascading tokens that lead to cascading errors.
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

#endif // !RK_LEXER_H
