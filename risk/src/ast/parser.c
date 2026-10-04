#include "risk/src/ast/core.h"
#include "risk/src/lex/core.h"
#include "std/flow/core.h"
#include "std/fmt/core.h"

#include "risk/src/ast/parser.h"
#include "risk/src/ast/power.h"
#include "risk/src/ast/msg.h"
#include "risk/src/ast/set.h"
#include "std/mem/bump.h"

// |================================================================================================|
// |> PARSER HELPERS                                                                                |

void parser_skip(Parser * parser) {
	for (;;) {
		LexToken token = lex_next(&parser->lexer);
		if (!LEX_IS_SPACES(token.tag)) {
			parser->previous = parser->peek;
			parser->peek = token;
			break;
		};
	};
};

void parser_skip_until_pivot(Parser * parser) {
	while (!lex_is_pivot(parser->peek.tag)) parser_skip(parser);
};

// |================================================================================================|
// |> PARSER INIT                                                                                   |

Parser parser_init(Src src, Fmt * fmt, Pages * nodes, Pages * tmp, u32 errors_limit) {
	Parser parser = (Parser){
		.fmt = fmt, .tmp = bump_init(tmp),
		.ast = ast_init(nodes, src), .lexer = lex_init(src.str_z),
		.error = {.count = 0, .limit = errors_limit},
		// .previous = LEX_TOKEN(0, 0, LEX_T_EOF), .peek = LEX_TOKEN(0, 0, LEX_T_EOF),
	};

	parser_skip(&parser); parser.previous = parser.peek;
	return parser;
};

// |================================================================================================|
// |> PARSER PRIVATE                                                                                |

AstIdx parser_sequence(Parser * parser, b8 file);
AstIdx parser_expr(Parser * parser, ParsePower min_power);

// |================================================================================================|
// |> PARSER EXPR                                                                                   |

inline AstIdx parser_sequence(Parser * parser, b8 file) {
	u32 pos_open = parser->peek.pos;
	LexTag close_token; AstBound bound;

	if (file) {
		close_token = LEX_T_EOF; bound = AST_B_EOF;
	} else switch (parser->peek.tag) {
		case LEX_T_PAREN_OPEN:   close_token = LEX_T_PAREN_CLOSE;   bound = AST_B_PAREN;   break;
		case LEX_T_BRACE_OPEN:   close_token = LEX_T_BRACE_CLOSE;   bound = AST_B_BRACE;   break;
		case LEX_T_BRACKET_OPEN: close_token = LEX_T_BRACKET_CLOSE; bound = AST_B_BRACKET; break;
		default: PANIC("unhandled tag [parser->peek.tag]");
	};

	if (close_token != LEX_T_EOF) parser_skip(parser);

	LexTag delim_token; AstDelim delim = AST_D_NONE;

	u32 count = 0;
	for (;;) {
		if (parser->peek.tag == close_token) break;
		if (!parser_error_can_accept(parser)) break;

		AstIdx expr = parser_expr(parser, PARSE_POWER_NIL);
		(void)bump_alloc(&parser->tmp, 1, expr); count += 1;

		if (delim == AST_D_NONE) {
			if (parser->peek.tag == LEX_T_COMMA) {
				delim_token = LEX_T_COMMA; delim = AST_D_COMMA;
			} else if (parser->peek.tag == LEX_T_SEMI)  {
				delim_token = LEX_T_SEMI; delim = AST_D_SEMI;
			} else {
				if (close_token != LEX_T_EOF) break;
				parser_expected_after(parser, parser->previous, S("`,` or `;`"));
				parser_skip(parser);
			};
		} else if (parser->peek.tag != delim_token) {
			if (close_token != LEX_T_EOF) break;
			while (parser->peek.tag != delim_token) parser_skip(parser);
		};

		parser_skip(parser);
	};

	u32 pos_close = parser->peek.pos;
	if (parser->peek.tag != close_token) {
		parser_expected(parser, parser->peek, lex_name(close_token));
		parser_skip_until_pivot(parser);
	} else {
		parser_skip(parser);
	};

	AstSeq seq = (AstSeq){
		.tag = AST_T_SEQ, .delim = delim, .bound = bound,
		.pos_open = pos_open, .pos_close = pos_close,
		.count = count,
	};

	AstIdx const seq_idx = ast_alloc(&parser->ast, seq);

	u64 const idxes_size = count * sizeof(AstIdx);
	uptr const start = parser->tmp.pos - idxes_size;

	AstIdx * _ptr = ast_node(&parser->ast, idxes_size);
	memcpy(_ptr, (void*)start, idxes_size);
	parser->tmp.pos = start;

	return seq_idx;
};

// |================================================================================================|
// |> PARSER EXPR                                                                                   |

AstIdx parser_alloc_poison(Parser * parser, LexToken token) {
	AstAtom poison = (AstAtom){.tag = AST_T_POISON, .pos = token.pos, .len = token.len};
	return ast_alloc(&parser->ast, poison);
};

AstIdx parser_expr(Parser * parser, ParsePower min_power) {
	if (!parser_error_can_accept(parser)) return parser_alloc_poison(parser, parser->peek);

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
			lhs = parser_sequence(parser, false);
			if (!lex_is_expr(parser->peek.tag)) break;

			AstSeq * seq = (void*)AST_IDX_AS_PTR(&parser->ast, lhs);
			seq->tag = AST_T_CALL_PREFIX;

			AstIdx caller_idx = ast_alloc(&parser->ast, lhs);
			AstIdx * caller_ptr = (AstIdx*)AST_IDX_AS_PTR(&parser->ast, caller_idx);

			*caller_ptr = parser_expr(parser, PARSE_POWER_PREFIX_NEG);
		} break;

		case LEX_T_PLUS: case LEX_T_MINUS: case LEX_T_BANG: case LEX_T_DOT: {
			u32 pos = parser->peek.pos; u16 len = parser->peek.len;

			AstTag tag; ParsePower power; switch (parser->peek.tag) {
				case LEX_T_PLUS:  tag = AST_T_UNA_POS; power = PARSE_POWER_PREFIX_POS; break;
				case LEX_T_MINUS: tag = AST_T_UNA_NEG; power = PARSE_POWER_PREFIX_NOT; break;
				case LEX_T_BANG:  tag = AST_T_UNA_NOT; power = PARSE_POWER_PREFIX_NEG; break;
				case LEX_T_DOT:   tag = AST_T_UNA_DOT; power = PARSE_POWER_PREFIX_DOT; break;
				default: PANIC("unhandled kind [parser->peek.tag]");
			};

			parser_skip(parser);
			AstUna una = (AstUna){.tag = tag, .pos = pos, .len = len, .node = parser_expr(parser, power)};
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
		if (!parser_error_can_accept(parser)) return parser_alloc_poison(parser, parser->peek);
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

				if (!lex_is_expr(parser->peek.tag)) {
					parser_expected_after(parser, infix, S("expression"));
					parser_skip_until_pivot(parser);
					return ast_alloc(&parser->ast, bin);
				};

				bin.rhs = parser_expr(parser, rhs_power);
				lhs = ast_alloc(&parser->ast, bin);
			} break;

			case LEX_T_PAREN_OPEN:
			case LEX_T_BRACE_OPEN:
			case LEX_T_BRACKET_OPEN: {
				if (min_power > PARSE_POWER_CALL_PARENS) return lhs;
				AstIdx const seq_idx = parser_sequence(parser, false);
				AstSeq * seq = (void*)AST_IDX_AS_PTR(&parser->ast, seq_idx);
				seq->tag = AST_T_CALL_POSTFIX; ast_alloc(&parser->ast, lhs);
				lhs = seq_idx;
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
	parser->ast.root = parser_sequence(parser, true);
	return parser->ast;
};

