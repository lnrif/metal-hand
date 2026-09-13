#include "prs/parser.h"
#include "core/types.h"
#include "lex/lexer.h"
#include "prs/ast.h"
#include "fmt/fmt.h"
#include "trc/assert.h"

#include <string.h>

////////////////////////////////////////////////////////////////
// parser common

Parser parser_init(Memory * mem, FmtVirt * out, Lex lex) {
	VirtStack pool = virt_stack_init(mem, .size = MB(64));
	VirtBump  tmp  = virt_bump_init(mem, .size = MB(1));
	PrsOpenVec opens = {.virt_vec = virt_vec_init(PrsOpenVec, mem, .size = KB(4))};

	return (Parser){
		.out = out, .raw = (Ast){.pool = pool, .src = lex.src},
		.tokens = lex.tokens, .at = 0,
		.opens = opens, .tmp = tmp,
	};
};

LexToken parser_token(Parser const * parser, usz idx) {
	ASSERT_DEBUG(parser != NULL, "expected [parser]");
	ASSERT_DEBUG(idx < parser->tokens.len, "[idx] out of bounds");
	return parser->tokens.ptr[idx];
};

LexToken parser_peek(Parser const * parser) {
	ASSERT_DEBUG(parser != NULL, "expected [parser]");
	ASSERT_DEBUG(parser->at < parser->tokens.len, "invalid invariant");
	return parser->tokens.ptr[parser->at];
};

void parser_skip(Parser * parser) {
	ASSERT_DEBUG(parser != NULL, "expected [parser]");
	ASSERT_DEBUG(parser->at < parser->tokens.len, "invalid invariant");
	if (parser->at != parser->tokens.len - 1) parser->at += 1;
};

LexToken parser_next(Parser * parser) {
	parser_skip(parser); return parser_peek(parser);
};

bool parser_eat(Parser * parser, /* optional */ LexToken * token, LexKind kind) {
	ASSERT_DEBUG(parser != NULL, "expected [parser]");
	ASSERT_DEBUG(parser->at < parser->tokens.len, "invalid invariant");

	LexToken const peek = parser_peek(parser);
	if (peek.kind != kind) return FALSE;

	if (token != NULL) *token = peek;
	parser_skip(parser);
	return TRUE;
};

////////////////////////////////////////////////////////////////
// ...

bool parser_open_eq(Parser const * parser, AstBound bound) {
	if (parser->opens.len == 0) return FALSE;
	return parser->opens.ptr[parser->opens.len - 1] == bound;
};

Str parser_token_lexeme(Parser const * parser, LexToken token) {
	return str_cut(parser->raw.src.str_z, token.pos, token.pos + lex_len(token));
};

////////////////////////////////////////////////////////////////
// errors

#define parser_unexpected(parser) parser_unexpected_ex(CALL, parser)
void parser_unexpected_ex(CallLoc call, Parser * parser) {
	LexToken token = parser_peek(parser);

	if (token.kind == LEX_EOF) {
		if (parser->at != 0) token = parser_token(parser, parser->at - 1);
		SrcDot const dot = src_eval_dot(parser->raw.src.str_z, token.pos);

		fmt_write(parser->out,
			FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("unexpected end fo file"),
			FMT_LINE,
			FMT_COLOR(FMT_GREY), FMT_LIT("| "),
			FMT_LOC(parser->raw.src.path.str_z, dot.row, dot.col),
			FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(call),
			FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
			FMT_LINE, FMT_COLOR(FMT_RESET),
		);

		return;
	};

	SrcDot const dot = src_eval_dot(parser->raw.src.str_z, token.pos);
	Str const lexeme = parser_token_lexeme(parser, token);

	fmt_write(parser->out,
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("unexpected token "),
		FMT_COLOR(FMT_RED), FMT_LIT("`"), FMT_STR(lexeme), FMT_LIT("`"), FMT_LINE,
		FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LOC(parser->raw.src.path.str_z, dot.row, dot.col),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_LINE, FMT_COLOR(FMT_RESET),
	);
};

#define parser_expected(parser, expect) parser_expected_ex(CALL, parser, expect)
void parser_expected_ex(CallLoc call, Parser * parser, Str expect) {
	LexToken token = parser_peek(parser);

	if (token.kind == LEX_EOF) {
		if (parser->at != 0) token = parser_token(parser, parser->at - 1);
		SrcDot const dot = src_eval_dot(parser->raw.src.str_z, token.pos);

		fmt_write(parser->out,
			FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
			FMT_COLOR(FMT_GREEN), FMT_STR(expect),
			FMT_COLOR(FMT_WHITE), FMT_LIT(", but found "),
			FMT_COLOR(FMT_RED), FMT_LIT("end fo file"),
			FMT_LINE,
			FMT_COLOR(FMT_GREY), FMT_LIT("| "),
			FMT_LOC(parser->raw.src.path.str_z, dot.row, dot.col),
			FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(call),
			FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
			FMT_LINE, FMT_COLOR(FMT_RESET),
		);
	
		return;
	};

	SrcDot const dot = src_eval_dot(parser->raw.src.str_z, token.pos);
	Str const lexeme = parser_token_lexeme(parser, token);

	fmt_write(parser->out,
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("expected "),
		FMT_COLOR(FMT_GREEN), FMT_STR(expect),
		FMT_COLOR(FMT_WHITE), FMT_LIT(", but found "),
		FMT_COLOR(FMT_RED), FMT_LIT("`"), FMT_STR(lexeme), FMT_LIT("`"), FMT_LINE,
		FMT_COLOR(FMT_GREY), FMT_LIT("| "),
		FMT_LOC(parser->raw.src.path.str_z, dot.row, dot.col),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_LINE, FMT_COLOR(FMT_RESET),
	);
};

#define parser_expected_after(parser, token, expected) parser_expected_after_ex(CALL, parser, token, expected)
void parser_expected_after_ex(CallLoc call, Parser * parser, LexToken token, Str expected) {
	SrcDot const dot = src_eval_dot(parser->raw.src.str_z, token.pos);
	Str const lexeme = parser_token_lexeme(parser, token);
	fmt_write(parser->out,
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("expected "), FMT_STR(expected), FMT_LIT(" after token "),
		FMT_COLOR(FMT_RED), FMT_LIT("`"), FMT_STR(lexeme), FMT_LIT("`"), FMT_LINE,
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC(parser->raw.src.path.str_z, dot.row, dot.col),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_LINE, FMT_COLOR(FMT_RESET),
	);
};

void parser_expected_closed(Parser * parser, LexToken open) {
	LexToken const after = parser_token(parser, parser->at - 1);
	SrcDot const   open_dot = src_eval_dot(parser->raw.src.str_z, open.pos);
	SrcDot const after_dot = src_eval_dot(parser->raw.src.str_z, after.pos);
	Str const lexeme = parser_token_lexeme(parser, after);

	Str close; switch (open.kind) {
		case LEX_PAREN_OPEN:   close = STR("`)`"); break;
		case LEX_BRACE_OPEN:   close = STR("`}`"); break;
		case LEX_BRACKET_OPEN: close = STR("`]`"); break;
		default: PANIC("unhandled (%u)", open.kind);
	};

	fmt_write(parser->out,
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("expected "),
		FMT_COLOR(FMT_GREEN), FMT_STR(close),
		FMT_COLOR(FMT_WHITE), FMT_LIT(" after "),
		FMT_COLOR(FMT_RED), FMT_LIT("`"), FMT_STR(lexeme), FMT_LIT("`"), FMT_LINE,
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC(parser->raw.src.path.str_z, after_dot.row, after_dot.col),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC(parser->raw.src.path.str_z, open_dot.row,  open_dot.col),
		FMT_COLOR(FMT_GREY), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_LINE
	);
};

////////////////////////////////////////////////////////////////
// checking peek

bool parser_is_expr_start(Parser const * parser) {
	switch (parser_peek(parser).kind) {
		// atom
		case LEX_IDENT: case LEX_NUMBER: case LEX_STRING:
		// prefix
		case LEX_PLUS: case LEX_MINUS: case LEX_BANG: case LEX_DOT:
		// calls & prefix
		case LEX_PAREN_OPEN: case LEX_BRACE_OPEN: case LEX_BRACKET_OPEN:
		// complex expr
		case LEX_KW_IF:
		// case LEX_KW_DEFER:
		// case LEX_KW_LOOP: case LEX_KW_FOR:
		// case LEX_KW_CONTINUE: case LEX_KW_RET:
		return TRUE;
		default: return FALSE;
	};
};

AstBound parser_bound_kind(LexKind kind) {
	switch (kind) {
		case LEX_PAREN_OPEN:   case LEX_PAREN_CLOSE:   return AST_B_PAREN;
		case LEX_BRACE_OPEN:   case LEX_BRACE_CLOSE:   return AST_B_BRACE;
		case LEX_BRACKET_OPEN: case LEX_BRACKET_CLOSE: return AST_B_BRACKET;
		default: PANIC("unhandled or invalid kind (%u)", kind);
	};
};

#define PARSER_PIVOTS(X) \
	X(LEX_KW_THEN) \
	X(LEX_KW_ELSE) \
	X(LEX_COMMA) \
	X(LEX_SEMI) \
	X(LEX_PAREN_CLOSE) \
	X(LEX_BRACE_CLOSE) \
	X(LEX_BRACKET_CLOSE) \
	X(LEX_EOF)

bool parser_is_pivot(Parser const * parser) {
	LexKind const kind = parser_peek(parser).kind;
	switch (kind) {
		#define X(token) case token:
		PARSER_PIVOTS(X) return TRUE;
		#undef X
		default: return FALSE;
	};
};

void parser_skip_until_pivot(Parser * parser) {
	while (!parser_is_pivot(parser)) parser_skip(parser);
};

// AstIdx parser_items_finish(
// 	Parser * parser, PrsItems items,
// 	AstDelim delim, AstBound bound,
// 	AstPos open, AstPos close
// ) {
// 	uptr const end = parser_tmp_mark(parser);
// 	usz const count = (end - items.start) / sizeof(PrsItem);
// 	ASSERT(count <= U16_MAX, "to many expression in sequence");
//
// 	{
// 		AstPos * delims = parser_alloc_ex(parser, count * sizeof(AstPos));
// 		ASSERT(delims != NULL, "OOM");
//
// 		AstIdx * exprs = parser_alloc_ex(parser, count * sizeof(AstIdx));
// 		ASSERT(exprs != NULL, "OOM");
//
// 		PrsItem * start = (PrsItem*)items.start;
// 		for (usz i = 0; i < count; i += 1) {
// 			PrsItem it = start[i];
// 			delims[i] = it.delim;
// 			exprs[i]  = it.index;
// 		};
//
// 		parser_tmp_load(parser, items.start);
// 	};
//
// 	AstSeqAlloc * seq = PARSER_ALLOC(parser, seq);
// 	ASSERT(seq != NULL, "OOM");
//
// 	*seq = AST_SEQ(delim, bound, (u16)count, open, close);
// 	return prs_idx(parser, seq);
// };

////////////////////////////////////////////////////////////////
// node alloc

#define parser_tmp_alloc(parser, count, T) virt_bump_take(&(parser)->tmp, count, T)
#define parser_tmp_mark(parser)            virt_bump_mark(&(parser)->tmp)
#define parser_tmp_load(parser, mark)      virt_bump_load(&(parser)->tmp, mark)

#define PARSER_ALLOC(parser, value) \
	({ \
		typeof(value) _parser_alloc_value = (value); \
		typeof(value) * _parser_alloc_ptr = parser_alloc_ex(parser, sizeof(_parser_alloc_value)); \
		ASSERT(_parser_alloc_ptr != NULL, "OOM while allocating AST node"); \
		*_parser_alloc_ptr = _parser_alloc_value; \
		PTR_TO_IDX(parser, _parser_alloc_ptr); \
	})

#define PARSER_ALLOC_INDEXES(parser, count) \
	({ \
		AstIdx * _parser_alloc_ptr = parser_alloc_ex(parser, (count) * sizeof(AstIdx)); \
		ASSERT(_parser_alloc_ptr != NULL, "OOM while allocating AST indexes"); \
		PTR_TO_IDX(parser, _parser_alloc_ptr); \
	})

static inline void * parser_alloc_ex(Parser * parser, usz len) {
	ASSERT_DEBUG(parser != NULL, "expected [parser]");
	ASSERT_DEBUG(len % 4 == 0, "invalid [len]");

	void * const at = (void*)virt_stack_raw(&parser->raw.pool, 1, len, 4);
	if (at == NULL || !virt_stack_commit(&parser->raw.pool)) return NULL;
	ASSERT_DEBUG(
		(uptr)at <= parser->raw.pool.end,
		"at: %llu, range: %llu..<%llu (pos: %llu, len: %llu, alloc: %llu, diff: %llu)",
		(uptr)at,
		parser->raw.pool.beg,
		parser->raw.pool.end,
		parser->raw.pool.pos,
		parser->raw.pool.end - parser->raw.pool.beg,
		parser->raw.pool.end - parser->raw.pool.pos,
		((uptr)at - parser->raw.pool.end)
	);
	return at;
};

#define PTR_TO_IDX(parser, ptr) AST_PTR_TO_IDX(&(parser)->raw, ptr)
#define IDX_TO_PTR(parser, idx) AST_IDX_TO_PTR(&(parser)->raw, idx)

// AstIdx parser_bound(Parser * parser) {
// 	LexToken t1 = parser_peek(parser);
//
// 	AstBound bound_kind; LexKind close_kind;
// 	switch (t1.kind) {
// 		case LEX_PAREN_OPEN:   bound_kind = AST_B_PAREN;   close_kind = LEX_PAREN_CLOSE;   break;
// 		case LEX_BRACE_OPEN:   bound_kind = AST_B_BRACE;   close_kind = LEX_BRACE_CLOSE;   break;
// 		case LEX_BRACKET_OPEN: bound_kind = AST_B_BRACKET; close_kind = LEX_BRACKET_CLOSE; break;
// 		default: PANIC("unhandled or invalid kind (%u)", t1.kind);
// 	}; parser_skip(parser);
//
// 	LexToken t2 = parser_peek(parser);
// 	if (t2.kind == close_kind) {
// 		parser_skip(parser);
//
// 		AstSeqAlloc * seq = PARSER_ALLOC(parser, seq);
// 		ASSERT(seq != NULL, "OOM");
//
// 		*seq = AST_SEQ(AST_D_NONE, bound_kind, 0, t1.pos, t2.pos);
// 		return prs_idx(parser, seq);
// 	};
//
// 	ASSERT(VIRT_VEC_PUSH(&parser->opens, bound_kind), "OOM, %llu", parser->opens.len);
//
// 	PrsItems items = parser_items(parser);
//
// 	LexKind kind = LEX_EOF;
// 	for (;;) {
// 		AstIdx const it = parser_infix(parser, PRS_POWER_NONE);
//
// 		LexToken peek = parser_peek(parser);
// 		if (peek.kind != LEX_COMMA && peek.kind != LEX_SEMI) {
// 			parser_items_push(parser, (PrsItem){.index = it, .delim = U32_MAX});
// 			break;
// 		};
//
// 		if (kind == LEX_EOF) kind = peek.kind;
//
// 		if (peek.kind == kind) {
// 			parser_items_push(parser, (PrsItem){.index = it, .delim = peek.pos});
// 			parser_skip(parser);
// 		} else {
// 			if (parser_is_pivot(parser)) break;
// 			ASSERT_DEBUG(parser->at >= 1, "");
//
// 			LexToken const token = parser->tokens.ptr[parser->at - 1];
// 			parser_expected_after(parser, token, kind == LEX_COMMA ? STR("`,`") : STR("`;`"));
// 		};
//
// 		peek = parser_peek(parser);
// 		if (peek.kind == close_kind || peek.kind == LEX_EOF) break;
// 	};
//
// 	t2 = parser_peek(parser);
// 	if (t2.kind == close_kind) {
// 		parser_skip(parser);
// 	} else {
// 		parser_expected_closed(parser, t1);
// 		parser_skip_until_pivot(parser);
// 	};
//
// 	ASSERT_DEBUG(parser->opens.len >= 1, "where paren...");
// 	ASSERT_DEBUG(parser_open_eq(parser, bound_kind), "invalid open on top");
// 	parser->opens.len -= 1;
//
// 	AstDelim delim; switch (kind) {
// 		case LEX_EOF:   delim = AST_D_NONE;  break;
// 		case LEX_COMMA: delim = AST_D_COMMA; break;
// 		case LEX_SEMI:  delim = AST_D_SEMI;  break;
// 		default: PANIC("%u", kind);
// 	};
//
// 	return parser_items_finish(parser, items, delim, bound_kind, t1.pos, t2.pos);
// };
//

// #define PRS_POWER_PREFIX(NAME) \
// 	PRS_POWER_##NAME##_PREFIX

#define PRS_POWER_LHS(NAME) \
	PRS_POWER_INFIX_##NAME##_LHS, \
	PRS_POWER_INFIX_##NAME##_RHS, \
	PRS_POWER_INFIX_##NAME##_FINISH

#define PRS_POWER_RHS(NAME) \
	PRS_POWER_INFIX_##NAME##_RHS, \
	PRS_POWER_INFIX_##NAME##_LHS, \
	PRS_POWER_INFIX_##NAME##_FINISH

// #define PRS_POWER_PREFIX_AS(AS, NAME) \
// 	PRS_POWER_INFIX_##NAME##_PREFIX = PRS_POWER_INFIX_##AS##_PREFIX

#define PRS_POWER_INFIX_AS(AS, NAME) \
	PRS_POWER_INFIX_##NAME##_LHS    = PRS_POWER_INFIX_##AS##_LHS, \
	PRS_POWER_INFIX_##NAME##_RHS    = PRS_POWER_INFIX_##AS##_RHS, \
	PRS_POWER_INFIX_##NAME##_FINISH = PRS_POWER_INFIX_##AS##_FINISH \

ENUM(PrsPower, u8) {
	PRS_POWER_NONE,

	// if a
	PRS_POWER_PREFIX_IF,
	// a then b
	PRS_POWER_LHS(THEN),
		// a elif b
		PRS_POWER_INFIX_AS(THEN, ELIF),
		// a else b
		PRS_POWER_INFIX_AS(THEN, ELSE),

	// a = b = c; a = (b = c)
	PRS_POWER_RHS(SET),
		// a := b := c; a := (b := c)
		PRS_POWER_INFIX_AS(SET, DEF),

	// a <| b <| c; a <| (b <| c)
	PRS_POWER_RHS(APPLY),
	// a :: b :: c; a :: (b :: c)
	PRS_POWER_RHS(EXTEND),
	// a => b => c; a => (b => c)
	PRS_POWER_RHS(IMPLIES),
	// a : b : c; a : (b : c)
	PRS_POWER_RHS(COLON),

	// a -> b { c } == (a -> b) { c }
	PRS_POWER_CALL_BRACES,

	// a -> b -> c; a -> (a -> b)
	PRS_POWER_RHS(ARROW),

	// a || b || c; (a || b) || c
	PRS_POWER_LHS(LOG_OR),
	// a && b && c; (a && b) && c
	PRS_POWER_LHS(LOG_AND),

	// a == b == c; (a == b) == c
	PRS_POWER_LHS(EQ),
		// a != b != c; (a != b) != c
		PRS_POWER_INFIX_AS(EQ, NEQ),

	// a < b < c; (a < b) < c
	PRS_POWER_LHS(LT),
		// a <= b <= c; (a <= b) <= c
		PRS_POWER_INFIX_AS(LT, LE),
		// a > b > c; (a > b) > c
		PRS_POWER_INFIX_AS(LT, GT),
		// a >= b >= c; (a >= b) >= c
		PRS_POWER_INFIX_AS(LT, GE),

	// // a < b
	// AST_K_BIN_LT,
	// // a <= b
	// AST_K_BIN_LE,
	// // a > b
	// 	AST_K_BIN_GT,
	// 	// a >= b
	// 	AST_K_BIN_GE,
	// 	// a == b
	// 	AST_K_BIN_EQ,
	// 	// a != b
	// 	AST_K_BIN_NEQ,
	//
	// 	// a || b
	// 	AST_K_BIN_LOG_OR,
	// 	// a && b
	// 	AST_K_BIN_LOG_AND,
	//

	// a + b + c; (a + b) + c
	PRS_POWER_LHS(ADD),
		// a - b - c; (a - b) - c
		PRS_POWER_INFIX_AS(ADD, SUB),

	// a * b * c == (a * b) * c
	PRS_POWER_LHS(MUL),
		// a / b / c == (a / b) / c
		PRS_POWER_INFIX_AS(MUL, DIV),

	// +a
	PRS_POWER_PREFIX_POS,
	// -a
	PRS_POWER_PREFIX_NEG = PRS_POWER_PREFIX_POS,
	// !a
	PRS_POWER_PREFIX_NOT = PRS_POWER_PREFIX_POS,
	// .a
	PRS_POWER_PREFIX_DOT = PRS_POWER_PREFIX_POS,

	// x * f()  => x * (f())
	PRS_POWER_CALL_PARENS,
	// x * f[]  => x * (f[])
	PRS_POWER_CALL_BRACKTES   = PRS_POWER_CALL_PARENS,
	// x * f.{} => x * (f.{})
	// x * f {} => (x * f) {}
	//      ^ no dot here
	PRS_POWER_CALL_DOT_BRACES = PRS_POWER_CALL_PARENS,

	// a.b.c; (a.b).c
	PRS_POWER_LHS(DOT),
		// a'b'c; (a'b)'c
		PRS_POWER_INFIX_AS(DOT, TICK),
};

////////////////////////////////////////////////////////////////
// api

AstIdx parser_infix(Parser * parser, PrsPower min_power);

AstIdx parser_seq(
	Parser * parser,
	LexToken open, LexToken * close,
	AstDelim * delim_kind, AstBound * bound_kind,
	u16 * count
);

////////////////////////////////////////////////////////////////
// infix

AstIdx parser_infix(Parser * parser, PrsPower min_power) {
	LexToken const token = parser_peek(parser);

	AstIdx lhs;
	switch (token.kind) {
		case LEX_IDENT: case LEX_NUMBER: case LEX_STRING: {
			AstKind kind; switch (token.kind) {
				case LEX_IDENT:  kind = AST_K_ATOM_IDT; break;
				case LEX_NUMBER: kind = AST_K_ATOM_NUM; break;
				case LEX_STRING: kind = AST_K_ATOM_STR; break;
				default: PANIC("unhandled kind (%u)", token.kind);
			};

			parser_skip(parser);
			AstAtomAlloc atom = AST_ATOM(kind, token);
			lhs = PARSER_ALLOC(parser, atom);
		} break;

		case LEX_PAREN_OPEN:
		case LEX_BRACE_OPEN:
		case LEX_BRACKET_OPEN: {
			parser_skip(parser);
			LexToken close; AstDelim delim_kind; AstBound bound_kind; u16 count;
			parser_seq(parser, token, &close, &delim_kind, &bound_kind, &count);
			
			
			lhs = PARSER_ALLOC(parser, AST_SEQ(delim_kind, bound_kind, count, token.pos, close.pos));
		} break;

		case LEX_PLUS: case LEX_MINUS: case LEX_BANG: case LEX_DOT:
		case LEX_KW_IF:
		{
			parser_skip(parser);

			AstKind kind; PrsPower power; switch (token.kind) {
				case LEX_PLUS:  kind = AST_K_UNA_POS; power = PRS_POWER_PREFIX_POS; break;
				case LEX_MINUS: kind = AST_K_UNA_NEG; power = PRS_POWER_PREFIX_NOT; break;
				case LEX_BANG:  kind = AST_K_UNA_NOT; power = PRS_POWER_PREFIX_NEG; break;
				case LEX_DOT:   kind = AST_K_UNA_DOT; power = PRS_POWER_PREFIX_DOT; break;
				case LEX_KW_IF: kind = AST_K_UNA_IF;  power = PRS_POWER_PREFIX_IF;  break;
				default: PANIC("unhandled kind (%u)", token.kind);
			};

			parser_infix(parser, power);
			lhs = PARSER_ALLOC(parser, AST_UNA(kind, token));
		} break;

		default: {
			parser_expected(parser, STR("expression"));
			parser_skip_until_pivot(parser);
			return PARSER_ALLOC(parser, AST_POISON(token));
		};
	};

	for (;;) {
		LexToken const infix = parser_peek(parser);

		switch (infix.kind) {
				// infix
			case LEX_PLUS:
			case LEX_MINUS:
			case LEX_STAR:
			case LEX_SLASH:
			case LEX_COLON:
			case LEX_EQ:
			case LEX_COLON_EQ:
			case LEX_EQ_GT:
			case LEX_MINUS_GT:
			case LEX_COLON_COLON:
			case LEX_LT_OR:
			case LEX_DOT:
			case LEX_TICK:

			case LEX_LT:
			case LEX_LT_EQ:
			case LEX_GT:
			case LEX_GT_EQ:

			case LEX_EQ_EQ:
			case LEX_NOT_EQ:

			case LEX_AND_AND:
			case LEX_OR_OR:

			case LEX_KW_THEN:
			case LEX_KW_ELIF:
			case LEX_KW_ELSE:
			// parens
			{
				#define PRS_POWER_INFIX(NAME) \
					lhs_power = PRS_POWER_INFIX_##NAME##_LHS; \
					rhs_power = PRS_POWER_INFIX_##NAME##_RHS

				AstKind kind; PrsPower lhs_power, rhs_power; switch (infix.kind) {
					case LEX_PLUS:         kind = AST_K_BIN_ADD;     PRS_POWER_INFIX(ADD);     break;
					case LEX_MINUS:        kind = AST_K_BIN_SUB;     PRS_POWER_INFIX(SUB);     break;
					case LEX_STAR:         kind = AST_K_BIN_MUL;     PRS_POWER_INFIX(MUL);     break;
					case LEX_SLASH:        kind = AST_K_BIN_DIV;     PRS_POWER_INFIX(DIV);     break;
					case LEX_COLON:        kind = AST_K_BIN_COLON;   PRS_POWER_INFIX(COLON);   break;
					case LEX_EQ:           kind = AST_K_BIN_SET;     PRS_POWER_INFIX(SET);     break;
					case LEX_COLON_EQ:     kind = AST_K_BIN_DEF;     PRS_POWER_INFIX(DEF);     break;
					case LEX_EQ_GT:        kind = AST_K_BIN_IMPLIES; PRS_POWER_INFIX(IMPLIES); break;
					case LEX_MINUS_GT:     kind = AST_K_BIN_ARROW;   PRS_POWER_INFIX(ARROW);   break;
					case LEX_COLON_COLON:  kind = AST_K_BIN_EXTEND;  PRS_POWER_INFIX(EXTEND);  break;
					case LEX_LT_OR:        kind = AST_K_BIN_APPLY;   PRS_POWER_INFIX(APPLY);   break;
					case LEX_DOT:          kind = AST_K_BIN_DOT;     PRS_POWER_INFIX(DOT);     break;
					case LEX_TICK:         kind = AST_K_BIN_TICK;    PRS_POWER_INFIX(TICK);    break;

					case LEX_LT:           kind = AST_K_BIN_LT;      PRS_POWER_INFIX(LT);      break;
					case LEX_LT_EQ:        kind = AST_K_BIN_LE;      PRS_POWER_INFIX(LE);      break;
					case LEX_GT:           kind = AST_K_BIN_GT;      PRS_POWER_INFIX(GT);      break;
					case LEX_GT_EQ:        kind = AST_K_BIN_GE;      PRS_POWER_INFIX(GE);      break;

					case LEX_EQ_EQ:        kind = AST_K_BIN_EQ;      PRS_POWER_INFIX(EQ);      break;
					case LEX_NOT_EQ:       kind = AST_K_BIN_NEQ;     PRS_POWER_INFIX(NEQ);     break;

					case LEX_OR_OR:        kind = AST_K_BIN_LOG_OR;  PRS_POWER_INFIX(LOG_OR);  break;
					case LEX_AND_AND:      kind = AST_K_BIN_LOG_AND; PRS_POWER_INFIX(LOG_AND); break;

					case LEX_KW_THEN:      kind = AST_K_BIN_THEN;    PRS_POWER_INFIX(THEN);    break;
					case LEX_KW_ELIF:      kind = AST_K_BIN_ELIF;    PRS_POWER_INFIX(ELIF);    break;
					case LEX_KW_ELSE:      kind = AST_K_BIN_ELSE;    PRS_POWER_INFIX(ELSE);    break;
					default: PANIC("unhandled kind (%u)", token.kind);
				};

				if (min_power > lhs_power) return lhs;

				parser_skip(parser);
				if (!parser_is_expr_start(parser)) {
					LexToken const t3 = parser_peek(parser);

					parser_expected_after(parser, infix, STR("expression"));
					parser_skip_until_pivot(parser);

					(void)PARSER_ALLOC(parser, AST_POISON(t3));
					return PARSER_ALLOC(parser, AST_BIN(kind, infix, lhs));
				};

				(void)parser_infix(parser, rhs_power);
				lhs = PARSER_ALLOC(parser, AST_BIN(kind, infix, lhs));
			} break;

			case LEX_PAREN_OPEN:
			case LEX_BRACE_OPEN:
			case LEX_BRACKET_OPEN: {
				PrsPower power; switch (infix.kind) {
					case LEX_PAREN_OPEN:   power = PRS_POWER_CALL_PARENS;   break;
					case LEX_BRACE_OPEN:   power = PRS_POWER_CALL_BRACES;   break;
					case LEX_BRACKET_OPEN: power = PRS_POWER_CALL_BRACKTES; break;
					default: PANIC("unhandled kind (%u)", token.kind);
				}; if (min_power > power) return lhs;

				parser_skip(parser);
				LexToken close; AstDelim delim_kind; AstBound bound_kind; u16 count;
				parser_seq(parser, infix, &close, &delim_kind, &bound_kind, &count);

				lhs = PARSER_ALLOC(parser, AST_CALL_POSTFIX(lhs, infix.pos, close.pos, delim_kind, bound_kind, count));
			} break;

			default: return lhs;
		};
	};
};

////////////////////////////////////////////////////////////////
// root

AstIdx parser_seq(
	Parser * parser,
	LexToken open, LexToken * close,
	AstDelim * delim_kind, AstBound * bound_kind,
	u16 * count
) {
	AstKind close_kind;
	switch (open.kind) {
		case LEX_PAREN_OPEN:   *bound_kind = AST_B_PAREN;   close_kind = LEX_PAREN_CLOSE;   break;
		case LEX_BRACE_OPEN:   *bound_kind = AST_B_BRACE;   close_kind = LEX_BRACE_CLOSE;   break;
		case LEX_BRACKET_OPEN: *bound_kind = AST_B_BRACKET; close_kind = LEX_BRACKET_CLOSE; break;
		case LEX_EOF:          *bound_kind = AST_B_EOF;     close_kind = LEX_EOF;           break;
		default: PANIC("invalid open (%s)", lex_token_name(open.kind).ptr);
	};

	LexKind delim = LEX_EOF;
	uptr const start = parser->tmp.pos;

	for (;;) {
		LexToken peek = parser_peek(parser);
		if (peek.kind == close_kind) {
			parser_skip(parser); *close = peek;
			break;
		};

		if (peek.kind == LEX_EOF) {
			parser_expected_closed(parser, open);
			*close = LEX_TOKEN(LEX_EOF, peek.pos, 0);
			break;
		};

		AstIdx expr = parser_infix(parser, PRS_POWER_NONE);
		ASSERT(
			virt_bump_alloc(&parser->tmp, expr, 1) != NULL,
			"OOM while allocating temporary node"
		);

		peek = parser_peek(parser);
		if (delim == LEX_EOF) {
			if (peek.kind == LEX_COMMA || peek.kind == LEX_SEMI) {
				delim = peek.kind; parser_skip(parser);
				continue;
			};
		} else if (peek.kind == delim) {
			parser_skip(parser);
			continue;
		};

		if (peek.kind == close_kind) {
			parser_skip(parser); *close = peek;
			break;
		};

		Str expected;
		switch (delim) {
			case LEX_COMMA: expected = STR("`,`");        break;
			case LEX_SEMI:  expected = STR("`;`");        break;
			case LEX_EOF:   expected = STR("`,` or `;`"); break;
			default: PANIC("unhandled delim (%u)", delim);
		}; parser_expected(parser, expected);

		if (parser_is_expr_start(parser)) continue;

		parser_skip_until_pivot(parser);
		peek = parser_peek(parser);
		if (delim == LEX_EOF) {
			if (peek.kind == LEX_COMMA || peek.kind == LEX_SEMI) {
				delim = peek.kind; parser_skip(parser);
				continue;
			};
		} else if (peek.kind == delim) {
			parser_skip(parser);
			continue;
		};

		if (peek.kind == close_kind) {
			parser_skip(parser); *close = peek;
		};

		break;
	};

	uptr const end = parser->tmp.pos;
	usz  const bytes = end - start;
	usz  const count_full = bytes / sizeof(AstIdx);

	ASSERT(count_full <= U16_MAX, "too many children nodes");
	*count = (u16)count_full;

	switch (delim) {
		case LEX_EOF:   *delim_kind = AST_D_NONE; break;
		case LEX_COMMA: *delim_kind = AST_D_COMMA; break;
		case LEX_SEMI:  *delim_kind = AST_D_SEMI;  break;
		default: PANIC("");
	};

	AstIdx indexes = PARSER_ALLOC_INDEXES(parser, *count);
	memcpy(IDX_TO_PTR(parser, indexes), (AstIdx*)start, bytes);
	virt_bump_load(&parser->tmp, start);

	return indexes;
};

AstIdx parser_root(Parser * parser) {
	LexToken close; AstDelim delim_kind; AstBound bound_kind; u16 count;
	(void)parser_seq(parser, LEX_TOKEN(LEX_EOF, 0, 0), &close, &delim_kind, &bound_kind, &count);
	return PARSER_ALLOC(parser, AST_ROOT(delim_kind, count));
};

// AST_SEQ(*delim, bound, (u16)count, open.pos, close_pos)
// 	PrsItems items = parser_items(parser);
// 	AstDelim delim = AST_D_NONE;
//
// 	LexToken const t1 = parser_peek(parser);
// 	if (t1.kind == LEX_EOF) goto done;
//
// 	LexKind kind = LEX_EOF;
// 	usz at = parser->at;
// 	for (;; at = parser->at) {
// 		AstIdx const it = parser_infix(parser, PRS_POWER_NONE);
// 		if (at == parser->at) parser_skip(parser);
//
// 		LexToken peek = parser_peek(parser);
// 		if (peek.kind != LEX_COMMA && peek.kind != LEX_SEMI && peek.kind != LEX_EOF) {
// 			parser_items_push(parser, (PrsItem){.index = it, .delim = U32_MAX});
// 			parser_expected(parser, STR("`,` or `;`"));
//
// 			continue;
// 		};
//
// 		if (kind == LEX_EOF) kind = peek.kind;
//
// 		if (peek.kind == kind) {
// 			parser_items_push(parser, (PrsItem){.index = it, .delim = peek.pos});
// 			parser_skip(parser);
// 		} else {
// 			if (parser_is_pivot(parser)) break;
// 			ASSERT_DEBUG(parser->at >= 1, "");
//
// 			LexToken const token = parser->tokens.ptr[parser->at - 1];
// 			parser_expected_after(parser, token, kind == LEX_COMMA ? STR("`,`") : STR("`;`"));
// 		};
//
// 		peek = parser_peek(parser);
// 		if (peek.kind == LEX_EOF) break;
// 	};
//
// 	switch (kind) {
// 		case LEX_EOF:   delim = AST_D_NONE;  break;
// 		case LEX_COMMA: delim = AST_D_COMMA; break;
// 		case LEX_SEMI:  delim = AST_D_SEMI;  break;
// 		default: PANIC("unhandled delim (%u)", kind);
// 	};
//
// done:
// 	return parser_items_finish(parser, items, delim, AST_D_NONE, U32_MAX, U32_MAX);

