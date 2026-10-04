#include "risk/src/ast/fmt.h"
#include "risk/src/ast/core.h"
#include "risk/src/ast/set.h"
#include "risk/src/src/core.h"
#include "std/flow/core.h"
#include "std/fmt/core.h"

#define DEPTH_STEP 3

void ast_fmt_ex(
	Fmt * fmt, Ast const * ast,
	AstIdx idx, u16 depth,
	u32 num, u8 width
);

typedef enum: u8 {
	AST_FMT_SEQ,
	AST_FMT_SEQ_PREFIX,
	AST_FMT_SEQ_POSTFIX,
} AstFmtSeq;

void ast_fmt_seq(
	Fmt * fmt, Ast const * ast,
	AstIdx idx, u16 depth, AstFmtSeq kind
) {
	uptr const head = AST_IDX_AS_PTR(ast, idx);
	AstSeq const * seq = (void*)head;

	u64 x = seq->count;
	u8 w = 0; for (;;) {
		w += 1;
		x /= 10;
		if (x == 0) break;
	};

	Str lex; switch (seq->delim) {
		case AST_D_NONE:  lex = S("a b ..."); break;
		case AST_D_COMMA: lex = S("a, b, ..."); break;
		case AST_D_SEMI:  lex = S("a; b; ..."); break;
		default: PANIC("unhandled kind [seq->delim]");
	};

	Str open; Str close; switch (seq->bound) {
		case AST_B_EOF:     open = S("<("); close = S(")>"); break;
		case AST_B_PAREN:   open = S( "("); close = S(")" ); break;
		case AST_B_BRACE:   open = S( "{ "); close = S(" }" ); break;
		case AST_B_BRACKET: open = S( "["); close = S("]" ); break;
		default: PANIC("unhandled kind [seq->bound]");
	};

	Str prefix; Str postfix; switch (kind) {
		case AST_FMT_SEQ: prefix = S(""); postfix = S(""); break;
		case AST_FMT_SEQ_PREFIX: prefix = S(""); postfix = S("f"); break;
		case AST_FMT_SEQ_POSTFIX: prefix = S("g"); postfix = S(""); break;
	};

	Str const pad = kind != AST_FMT_SEQ && seq->bound == AST_B_BRACE ? S(" ") : S("");
	FMT(fmt,
		FMT_ORANGE,
		FMT_STR(prefix), FMT_STR(pad),
		FMT_STR(open), FMT_STR(lex), FMT_STR(close),
		FMT_STR(pad), FMT_STR(postfix),
	);

	FMT(fmt, FMT_LIT("\n"));
	if (kind != AST_FMT_SEQ) ast_fmt_ex(fmt, ast, seq->idx[seq->count], depth + 1, 0, w);

	for (u32 i = 0; i < seq->count; i += 1) {
		ast_fmt_ex(fmt, ast, seq->idx[i], depth + 1, i + (kind != AST_FMT_SEQ ? 1 : 0), w);
	};

	// FMT(fmt, FMT_REPEAT(' ', depth * DEPTH_STEP));
};

void ast_fmt_ex(
	Fmt * fmt, Ast const * ast,
	AstIdx idx, u16 depth,
	u32 num, u8 width
) {
	uptr const head = AST_IDX_AS_PTR(ast, idx);
	AstTag const * tag = (void*)head;

	FMT(fmt,
		FMT_REPEAT(' ', DEPTH_STEP * depth), FMT_BLUE,
		FMT_LIT("["), FMT_U64(num, .digits = width), FMT_LIT("] ")
	);

	switch (*tag) {
		case AST_T_POISON: {
			FMT(fmt, FMT_RED, FMT_LIT("poison"), FMT_LIT("\n"));
		} break;

		case AST_T_ATOM_IDENT:
		case AST_T_ATOM_NUMBER:
		case AST_T_ATOM_STRING: {
			AstAtom const * atom = (void*)head;
			SrcDot dot = src_dot(ast->src.str, SRC_DOT_NIL, atom->pos);

			// Str name; switch (atom->tag) {
			// 	case AST_T_ATOM_IDENT: name = S("ident"); break;
			// 	case AST_T_ATOM_NUMBER: name = S("number"); break;
			// 	case AST_T_ATOM_STRING: name = S("string"); break;
			// 	default: PANIC("unhandled atom kind [atom->tag]");
			// };

			Str lex = str_sub(ast->src.str, atom->pos, atom->pos + atom->len);
			FMT(fmt,
				FMT_GREEN, FMT_LIT("`"), FMT_STR(lex), FMT_LIT("` "),
				// FMT_MAGENTA, FMT_STR(name), FMT_LIT(" "),
				FMT_LOC_NO_ARROW(ast->src.path, dot.row, dot.col),
			);
		} break;

		case AST_T_UNA_POS:
		case AST_T_UNA_NEG:
		case AST_T_UNA_NOT:
		case AST_T_UNA_DOT:
		case AST_T_UNA_REF:
		case AST_T_UNA_DEREF:
		{
			AstUna const * una = (void*)head;
			Str lex = str_sub(ast->src.str, una->pos, una->pos + una->len);

			FMT(fmt,
				FMT_ORANGE, FMT_LIT("`"), FMT_STR(lex), FMT_LIT("` "),
				// FMT_GREY, FMT_LIT("("),
				FMT_LIT("\n"),
			);

			ast_fmt_ex(fmt, ast, una->node, depth + 1, 0, 1);
			// FMT(fmt, FMT_REPEAT(' ', depth * DEPTH_STEP));
			// FMT(fmt, FMT_GREY, FMT_LIT(")"), FMT_LIT("\n"));
		} break;

#define X(TOKEN, NODE) case AST_T_BIN_##NODE:
		// case AST_T_BIN_ADD:
		// case AST_T_BIN_SUB:
		// case AST_T_BIN_MUL:
		// case AST_T_BIN_DIV:
		//
		// case AST_T_BIN_DOT:
		// case AST_T_BIN_TICK:
		//
		// case AST_T_BIN_COLON:
		// case AST_T_BIN_SET:
		// case AST_T_BIN_DEF:
		//
		// case AST_T_BIN_ARROW:
		// case AST_T_BIN_IMPLIES:
		// case AST_T_BIN_EXTEND:
		// case AST_T_BIN_APPLY:
		//
		// case AST_T_BIN_THEN:
		// case AST_T_BIN_ELIF:
		// case AST_T_BIN_ELSE:

		LEX_AST_INFIX(X)
#undef X
		{
			AstBin const * bin = (void*)head;

			// Str lex = str_sub(ast->src.str, bin->pos, bin->pos + bin->len);
			SrcDot dot = src_dot(ast->src.str, SRC_DOT_NIL, bin->pos);
			FMT(fmt,
				FMT_ORANGE, FMT_LIT("`"), FMT_STR(ast_name(bin->tag)), FMT_LIT("` "),
				FMT_LOC_NO_ARROW(ast->src.path, dot.row, dot.col),
			);

			ast_fmt_ex(fmt, ast, bin->lhs, depth + 1, 0, 1);
			ast_fmt_ex(fmt, ast, bin->rhs, depth + 1, 1, 1);
		} break;

		case AST_T_SEQ:          ast_fmt_seq(fmt, ast, idx, depth, AST_FMT_SEQ); break;
		case AST_T_CALL_PREFIX:  ast_fmt_seq(fmt, ast, idx, depth, AST_FMT_SEQ_PREFIX); break;
		case AST_T_CALL_POSTFIX: ast_fmt_seq(fmt, ast, idx, depth, AST_FMT_SEQ_POSTFIX); break;

		// case AST_T_CALL: {
		// 	AstCall const * call = (void*)head;
		// 	i16 const w = (head->with.seq.count == 0) ? 0 : u64_len(head->with.seq.count - 1, 10);
		//
		// 	Str lex; switch (AST_SEQ_DELIM(call)) {
		// 		case AST_D_NONE:  lex = STR("call-postfix:none"); break;
		// 		case AST_D_COMMA: lex = STR("call-postfix:comma"); break;
		// 		case AST_D_SEMI:  lex = STR("call-postfix:semi");  break;
		// 		default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(call));
		// 	};
		//
		// 	Str open; Str close; switch (AST_SEQ_BOUND(call)) {
		// 		case AST_B_EOF:     open = STR("<{"); close = STR("}>"); break;
		// 		case AST_B_PAREN:   open = STR("(");  close = STR(")"); break;
		// 		case AST_B_BRACE:   open = STR("{");  close = STR("}"); break;
		// 		case AST_B_BRACKET: open = STR("[N");  close = STR("]"); break;
		// 		default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(call));
		// 	};
		//
		// 	fmt_write(fmt,
		// 		FMT_COLOR(FMT_ORANGE), FMT_STR(lex),
		// 		// FMT_COLOR(FMT_RED), FMT_U64(seq->node_seq.delim_and_bound, .width = 10, .num = FMT_NUM_BIN | FMT_NUM_FILL),
		// 		FMT_COLOR(FMT_GREY), FMT_LIT(" "), FMT_STR(open), FMT_LINE,
		// 	);
		//
		// 	ast_fmt_ex(fmt, ast, call->fun, depth + 1, 0, w);
		//
		// 	for (u16 i = 0;; i += 1) {
		// 		if (call->node_seq.count == 0 || i >= call->node_seq.count - 1) break;
		// 		ast_fmt_ex(fmt, ast, call->exprs[i], depth + 1, i + 1, w);
		// 		// fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LINE);
		// 	};
		//
		// 	if (call->node_seq.count >= 1) {
		// 		u16 const i = call->node_seq.count - 1;
		// 		ast_fmt_ex(fmt, ast, call->exprs[i], depth + 1, i + 1, w);
		// 	};
		//
		// 	fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
		// 	fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_STR(close), FMT_LINE);
		// } break;

		// case AST_T_ROOT: {
		// 	AstRoot const * seq = (void*)head;
		// 	i16 const w = (head->with.seq.count == 0) ? 0 : u64_len(head->with.seq.count - 1, 10);
		//
		// 	Str lex; switch (AST_SEQ_DELIM(seq)) {
		// 		case AST_D_NONE:  lex = STR("root.none"); break;
		// 		case AST_D_COMMA: lex = STR("root.comma"); break;
		// 		case AST_D_SEMI:  lex = STR("root.semi"); break;
		// 		default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(seq));
		// 	};
		//
		// 	Str open; Str close; switch (AST_SEQ_BOUND(seq)) {
		// 		case AST_B_EOF:     open = STR("<("); close = STR(")>"); break;
		// 		case AST_B_PAREN:   open =  STR("("); close = STR(")"); break;
		// 		case AST_B_BRACE:   open =  STR("{"); close = STR("}"); break;
		// 		case AST_B_BRACKET: open =  STR("[N"); close = STR("]"); break;
		// 		default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(seq));
		// 	};
		//
		// 	fmt_write(fmt,
		// 		FMT_COLOR(FMT_BLUE), FMT_STR(lex),
		// 		// FMT_COLOR(FMT_RED), FMT_U64(seq->node_seq.delim_and_bound, .width = 10, .num = FMT_NUM_BIN | FMT_NUM_FILL),
		// 		FMT_COLOR(FMT_GREY), FMT_LIT(" "), FMT_STR(open), FMT_LINE,
		// 	);
		//
		// 	for (u16 i = 0;; i += 1) {
		// 		if (seq->node_seq.count == 0 || i >= seq->node_seq.count - 1) break;
		// 		ast_fmt_ex(fmt, ast, seq->exprs[i], depth + 1, i, w);
		// 		// fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LINE);
		// 	};
		//
		// 	if (seq->node_seq.count >= 1) {
		// 		u16 const i = seq->node_seq.count - 1;
		// 		ast_fmt_ex(fmt, ast, seq->exprs[i], depth + 1, i, w);
		// 	};
		//
		// 	fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
		// 	fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_STR(close), FMT_LINE);
		//
		// } break;
		default: PANIC("unhandled kind [*tag]");
	};
};

void ast_fmt(Fmt * fmt, Ast const * ast, AstIdx idx) {
	u32 const DELIM_SIZE = 36;

	FMT(fmt,
		FMT_YELLOW, FMT_REPEAT('=', DELIM_SIZE), FMT_LIT(" AST "), FMT_REPEAT('=', DELIM_SIZE),
		FMT_LIT("\n\n"),
	);

	ast_fmt_ex(fmt, ast, idx, 0, 0, 1);

	FMT(fmt,
		FMT_LIT("\n"),
		FMT_YELLOW, FMT_REPEAT('=', DELIM_SIZE * 2 + S(" LEX ").len),
		FMT_LIT("\n\n"),
	);
};


