#include "lex/lexer.h"
#include "assert.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// token

u32 lex_len(LexToken token) {
	return (token.kind_and_len >> 8) & 0xFFFFFF;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// token kind

Str lex_token_name(LexKindTypo kind) {
	switch (kind) {
	case LEX_EOF: return STR("eof");
	case LEX_ILLEGAL: return STR("illegal");

	case LEX_IDENT: return STR("ident");
	case LEX_NUMBER: return STR("number");
	case LEX_STRING: return STR("string");
	case LEX_STRING_LINE: return STR("string-line");
	case LEX_STRING_UNTERMINATED: return STR("unterminated");

	case LEX_PLUS_PERCENT_EQ: return STR("'+%='");
	case LEX_PLUS_EQ: return STR("'+='");

	case LEX_PLUS_PERCENT: return STR("'+%'");

	case LEX_PLUS_PLUS: return STR("'++'");
	case LEX_PLUS: return STR("'+'");

	case LEX_MINUS_PERCENT_EQ: return STR("'-%='");
	case LEX_MINUS_EQ: return STR("'-='");

	case LEX_MINUS_PERCENT: return STR("'-%'");

	case LEX_MINUS_MINUS_MINUS: return STR("'---'");
	// case LEX_MINUS_MINUS: return STR("'--'");
	case LEX_MINUS_GT: return STR("'->'");
	case LEX_MINUS: return STR("'-'");

	case LEX_STAR_PERCENT_EQ: return STR("'*%='");
	case LEX_STAR_EQ: return STR("'*='");

	case LEX_STAR_PERCENT: return STR("'*%'");

	case LEX_STAR_STAR: return STR("'**'");
	case LEX_STAR: return STR("'*'");

	case LEX_SLASH_EQ: return STR("'/='");
	case LEX_SLASH: return STR("'/'");

	case LEX_PERCENT: return STR("'%'");

	case LEX_AND_AND: return STR("'&&'");
	case LEX_AND: return STR("'&'");

	case LEX_OR_OR: return STR("'||'");
	case LEX_OR: return STR("'|'");

	case LEX_CARET: return STR("'^'");

	case LEX_NOT_EQ: return STR("'!='");
	case LEX_EQ_EQ: return STR("'=='");
	case LEX_EQ_GT: return STR("'=>'");
	case LEX_EQ: return STR("'='");

	case LEX_LT_EQ: return STR("'<='");
	case LEX_LT_OR: return STR("'<|'");
	case LEX_LT_TILDE: return STR("'<~'");
	case LEX_LT_MINUS: return STR("'<-'");
	case LEX_LT: return STR("'<'");

	case LEX_GT_EQ: return STR("'>='");
	case LEX_GT: return STR("'>'");

	case LEX_TILDE_TILDE: return STR("'~~'");
	case LEX_TILDE_GT: return STR("'~>'");
	case LEX_TILDE: return STR("'~'");

	case LEX_DOT_DOT_EQ: return STR("'..='");
	case LEX_DOT_DOT_LT: return STR("'..<'");
	case LEX_DOT_DOT_DOT: return STR("'...'");
	case LEX_DOT_DOT: return STR("'..'");
	case LEX_DOT_STAR: return STR("'.*'");
	case LEX_DOT: return STR("'.'");

	case LEX_COLON_COLON: return STR("'::'");
	case LEX_COLON_EQ: return STR("':='");
	case LEX_COLON: return STR("':'");

	case LEX_SEMI: return STR("';'");
	case LEX_COMMA: return STR("','");
	case LEX_TICK: return STR("'''");

	case LEX_BANG_BANG: return STR("'!!'");
	case LEX_BANG: return STR("'!'");

	case LEX_QMARK_QMARK_QMARK: return STR("'\?\?\?'");
	case LEX_QMARK_QMARK: return STR("'\?\?'");
	case LEX_QMARK: return STR("'?'");

	case LEX_AT: return STR("'@'");

	case LEX_PAREN_OPEN: return STR("'('");
	case LEX_PAREN_CLOSE: return STR("')'");
	case LEX_BRACKET_OPEN: return STR("'['");
	case LEX_BRACKET_CLOSE: return STR("']'");
	case LEX_BRACE_OPEN: return STR("'{'");
	case LEX_BRACE_CLOSE: return STR("'}'");

	////////////////////////////////
	// keywords

	case LEX_KW_IF: return STR("keyword 'if'");
	case LEX_KW_THEN: return STR("keyword 'then'");
	case LEX_KW_ELIF: return STR("keyword 'elif'");
	case LEX_KW_ELSE: return STR("keyword 'else'");

	case LEX_KW_FOR: return STR("keyword 'for'");
	case LEX_KW_LOOP: return STR("keyword 'loop'");

	case LEX_KW_CONTINUE: return STR("keyword 'continue'");
	case LEX_KW_DEFER: return STR("keyword 'defer'");
	case LEX_KW_RET: return STR("keyword 'ret'");

	////////////////////////////////
	// extended kinds

	case LEX_EXT_COMMENT: return STR("comment");
	case LEX_EXT_SPACE: return STR("space");

	// case LEX_EXT_QUOTES: return STR("string-quote");
	// case LEX_EXT_STRING_LINE: return STR("string-line");
	// case LEX_EXT_STRING_NEWLINE: return STR("string-newline");

	////////////////////////////////
	// typo kinds

	case LEX_TYPO_SLASH_COMMENT: return STR("c-like-comment");
	case LEX_TYPO_QUOTES: return STR("typo '`...`'");
	case LEX_TYPO_QUOTES_UNTERMINATED: return STR("typo '`...'");

	case LEX_TYPO_DOLLAR: return STR("typo '$...'");
	case LEX_TYPO_BACK_SLASH: return STR("typo '\\...'");

	default: PANIC("typo 'invalid/unhandled kind (%u)'", kind);
	};
};

