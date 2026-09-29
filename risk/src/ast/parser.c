#include "std/flow/core.h"
#include "std/fmt/core.h"

#include "risk/src/ast/parser.h"
#include "risk/src/ast/power.h"
#include "risk/src/ast/msg.h"
#include "risk/src/ast/set.h"

// |================================================================================================|
// |> PARSER HELPERS                                                                                |

void parser_skip(Parser * parser) {
	for (;;) {
		parser->peek = lex_next(&parser->lexer);
		if (!LEX_IS_SPACES(parser->peek.tag)) break;
	};
};

void parser_skip_until_pivot(Parser * parser) {
	while (!lex_is_pivot(parser->peek.tag)) parser_skip(parser);
};

// |================================================================================================|
// |> PARSER INIT                                                                                   |

Parser parser_init(Src src, Fmt * fmt, Pages * ast) {
	Parser parser = (Parser){
		.fmt = fmt,
		.ast = ast_init(ast, src), .lexer = lex_init(src.str_z),
	};

	parser_skip(&parser);
	return parser;
};

// |================================================================================================|
// |> PARSER INFIX                                                                                  |

AstIdx parser_infix(Parser * parser, ParsePower min_power) {
	UNUSED(min_power);

	// TODO
	// if (parser->error) return AST_IDX_NIL;

	AstIdx lhs;
	switch (parser->peek.tag) {
		case LEX_T_IDENT: case LEX_T_NUMBER: case LEX_T_STRING: {
			AstTag tag; switch (parser->peek.tag) {
				case LEX_T_IDENT:  tag = AST_T_ATOM_IDENT;  break;
				case LEX_T_NUMBER: tag = AST_T_ATOM_NUMBER; break;
				case LEX_T_STRING: tag = AST_T_ATOM_STRING; break;
				default: PANIC("unhandled tag [parser->peek.tag]");
			};
			AstAtom atom = (AstAtom){.tag = tag, .pos = parser->peek.pos, .len = parser->peek.len};
			lhs = ast_alloc(&parser->ast, atom);
			parser_skip(parser);
		} break;

		case LEX_T_PAREN_OPEN:
		case LEX_T_BRACE_OPEN:
		case LEX_T_BRACKET_OPEN: {
			u32 pos_open = parser->peek.pos;
			LexTag close_kind; AstBound bound;
			switch (parser->peek.tag) {
				case LEX_T_PAREN_OPEN:   close_kind = LEX_T_PAREN_CLOSE;   bound = AST_B_PAREN;   break;
				case LEX_T_BRACE_OPEN:   close_kind = LEX_T_BRACE_CLOSE;   bound = AST_B_BRACE;   break;
				case LEX_T_BRACKET_OPEN: close_kind = LEX_T_BRACKET_CLOSE; bound = AST_B_BRACKET; break;
				default: PANIC("unhandled tag [parser->peek.tag]");
			};

			parser_skip(parser);
			if (parser->peek.tag != close_kind) {
				lhs = parser_infix(parser, PARSE_POWER_NIL);
			} else {
				AstSeq seq = (AstSeq){
					.tag = AST_T_SEQ,
					.delim = AST_D_NONE,
					.bound = bound,
					.pos_open = pos_open,
					.pos_close = parser->peek.pos,
					.count = 0,
				};
				lhs = ast_alloc(&parser->ast, seq);
			};

			if (parser->peek.tag != close_kind) {
				parser_expected(parser, parser->peek, lex_name(close_kind));
				parser_skip_until_pivot(parser);
			} else {
				parser_skip(parser);
			};
		} break;

		case LEX_T_PLUS: case LEX_T_MINUS: case LEX_T_BANG: case LEX_T_DOT: case LEX_T_COLON: {
			u32 pos = parser->peek.pos; u16 len = parser->peek.len;

			AstTag tag; ParsePower power; switch (parser->peek.tag) {
				case LEX_T_PLUS:  tag = AST_T_UNA_POS; power = PARSE_POWER_PREFIX_POS; break;
				case LEX_T_MINUS: tag = AST_T_UNA_NEG; power = PARSE_POWER_PREFIX_NOT; break;
				case LEX_T_BANG:  tag = AST_T_UNA_NOT; power = PARSE_POWER_PREFIX_NEG; break;
				case LEX_T_DOT:   tag = AST_T_UNA_DOT; power = PARSE_POWER_PREFIX_DOT; break;
				default: PANIC("unhandled kind [parser->peek.tag");
			};

			parser_skip(parser);
			AstUna una = (AstUna){.tag = tag, .pos = pos, .len = len, .node = parser_infix(parser, power)};
			lhs = ast_alloc(&parser->ast, una);
		} break;

		default: {
			AstAtom poison = (AstAtom){.tag = AST_T_POISON, .pos = parser->peek.pos, .len = parser->peek.len};
			parser_expected(parser, parser->peek, S("expression"));
			parser_skip_until_pivot(parser);
			return ast_alloc(&parser->ast, poison);
		};
	};

	for (;;) {
		LexToken const infix = parser->peek;
		
		switch (infix.tag) {
			#define X(TOKEN, NODE) case LEX_T_##TOKEN:
			LEX_AST_INFIX(X)
			#undef X
			{
				AstTag tag; ParsePower lhs_power, rhs_power; switch (infix.tag) {
					#define X(TOKEN, NODE) \
						case LEX_T_##TOKEN: { \
							tag = AST_T_BIN_##NODE; \
							lhs_power = PARSE_POWER_INFIX_##NODE##_LHS; \
							rhs_power = PARSE_POWER_INFIX_##NODE##_RHS; \
						} break;
					LEX_AST_INFIX(X)
					#undef X
					default: PANIC("unhandled tag [infix.tag]");
				};

				if (min_power > lhs_power) return lhs;

				AstBin bin = (AstBin){.tag = tag, .pos = infix.pos, .len = infix.len, .lhs = lhs, .rhs = AST_IDX_NIL};
				parser_skip(parser);

				if (!lex_is_pivot(parser->peek.tag)) {
					parser_expected_after(parser, infix, S("expression"));
					parser_skip_until_pivot(parser);
					return ast_alloc(&parser->ast, bin);
				};

				bin.rhs = parser_infix(parser, rhs_power);
				lhs = ast_alloc(&parser->ast, bin);
			} break;

			// case LEX_T_PAREN_OPEN:
			// case LEX_T_BRACE_OPEN:
			// case LEX_T_BRACKET_OPEN: {
			// 	ParsePower power; switch (parser->peek.tag) {
			// 		case LEX_T_PAREN_OPEN:   power = PARSE_POWER_CALL_PARENS;   break;
			// 		case LEX_T_BRACE_OPEN:   power = PARSE_POWER_CALL_BRACES;   break;
			// 		case LEX_T_BRACKET_OPEN: power = PARSE_POWER_CALL_BRACKTES; break;
			// 		default: PANIC("unhandled kind [parser->peek]");
			// 	};
			//
			// 	if (min_power > power) return lhs;
			// 	parser_skip(parser);
			//
			// 	LexToken close; AstDelim delim_kind; AstBound bound_kind; u16 count;
			// 	parser_seq(parser, infix, &close, &delim_kind, &bound_kind, &count);
			//
			// 	lhs = PARSER_ALLOC(parser, AST_CALL_POSTFIX(lhs, infix.pos, close.pos, delim_kind, bound_kind, count));
			// } break;

			default: return lhs;
		};
	};

	return lhs;
};

Ast parser_parse(Parser * parser) {
	parser->ast.root = parser_infix(parser, PARSE_POWER_NIL);
	return parser->ast;
};

