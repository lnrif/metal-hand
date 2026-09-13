#include "prs/ast.h"
#include "fmt/fmt.h"
#include "trc/assert.h"

StrZ ast_name(AstKind kind) {
	switch (kind) {
		case AST_K_POISON: return STR("poison");

		case AST_K_ATOM_IDT: return STR("ident");
		case AST_K_ATOM_NUM: return STR("number");
		case AST_K_ATOM_STR: return STR("string");

		case AST_K_UNA_POS: return STR("+a");
		case AST_K_UNA_NEG: return STR("-a");
		case AST_K_UNA_NOT: return STR("!a");
		case AST_K_UNA_DOT: return STR(".a");
		case AST_K_UNA_DEREF: return STR("a.*");
		case AST_K_UNA_REF: return STR("&a");

		case AST_K_UNA_IF: return STR("if a");

		case AST_K_BIN_ADD: return STR("a + b");
		case AST_K_BIN_SUB: return STR("a - b");
		case AST_K_BIN_MUL: return STR("a * b");
		case AST_K_BIN_DIV: return STR("a / b");

		case AST_K_BIN_DOT: return STR("a.b");
		case AST_K_BIN_TICK: return STR("a'b");

		case AST_K_BIN_COLON: return STR("a: b");
		case AST_K_BIN_SET: return STR("a = b");
		case AST_K_BIN_DEF: return STR("a := b");

		case AST_K_BIN_IMPLIES: return STR("a => b");
		case AST_K_BIN_ARROW: return STR("a -> b");
		case AST_K_BIN_EXTEND: return STR("a :: b");
		case AST_K_BIN_APPLY: return STR("a <| b");

		case AST_K_BIN_LT: return STR("a < b");
		case AST_K_BIN_LE: return STR("a <= b");
		case AST_K_BIN_GT: return STR("a > b");
		case AST_K_BIN_GE: return STR("a >= b");

		case AST_K_BIN_EQ: return STR("a == b");
		case AST_K_BIN_NEQ: return STR("a != b");

		case AST_K_BIN_LOG_OR: return STR("a || b");
		case AST_K_BIN_LOG_AND: return STR("a && b");

		case AST_K_BIN_THEN: return STR("a then b");
		case AST_K_BIN_ELIF: return STR("a elif b");
		case AST_K_BIN_ELSE: return STR("a else b");

		case AST_K_SEQ: return STR("seq");
		case AST_K_CALL_PREFIX:  return STR("call.prefix");
		case AST_K_CALL_POSTFIX: return STR("call.postfix");
		case AST_K_ROOT: return STR("root");

		default: PANIC("invalid node kind (%u)", kind);
	};
};

StrZ ast_tag(AstKind kind) {
	switch (kind) {
		case AST_K_POISON: return STR("poison");

		case AST_K_ATOM_IDT: return STR("ident");
		case AST_K_ATOM_NUM: return STR("number");
		case AST_K_ATOM_STR: return STR("string");

		case AST_K_UNA_POS: return STR("pos");
		case AST_K_UNA_NEG: return STR("neg");
		case AST_K_UNA_NOT: return STR("not");
		case AST_K_UNA_DOT: return STR("dot");
		case AST_K_UNA_DEREF: return STR("deref");
		case AST_K_UNA_REF: return STR("ref");

		case AST_K_UNA_IF: return STR("if");

		case AST_K_BIN_ADD: return STR("add");
		case AST_K_BIN_SUB: return STR("sub");
		case AST_K_BIN_MUL: return STR("mul");
		case AST_K_BIN_DIV: return STR("div");

		case AST_K_BIN_DOT: return STR("path");
		case AST_K_BIN_TICK: return STR("tick");

		case AST_K_BIN_COLON: return STR("colon");
		case AST_K_BIN_SET: return STR("set");
		case AST_K_BIN_DEF: return STR("def");

		case AST_K_BIN_IMPLIES: return STR("implies");
		case AST_K_BIN_ARROW: return STR("arrow");
		case AST_K_BIN_EXTEND: return STR("extend");
		case AST_K_BIN_APPLY: return STR("apply");

		case AST_K_BIN_LT: return STR("lt");
		case AST_K_BIN_LE: return STR("le");
		case AST_K_BIN_GT: return STR("gt");
		case AST_K_BIN_GE: return STR("ge");

		case AST_K_BIN_EQ: return STR("eq");
		case AST_K_BIN_NEQ: return STR("neq");

		case AST_K_BIN_LOG_OR: return STR("log-or");
		case AST_K_BIN_LOG_AND: return STR("log-and");

		case AST_K_BIN_THEN: return STR("then");
		case AST_K_BIN_ELIF: return STR("elif");
		case AST_K_BIN_ELSE: return STR("else");

		case AST_K_SEQ: return STR("seq");
		case AST_K_CALL_PREFIX:  return STR("call.prefix");
		case AST_K_CALL_POSTFIX: return STR("call.postfix");
		case AST_K_ROOT: return STR("root");

		default: PANIC("invalid node kind (%u)", kind);
	};
};

void * ast_idx_to_ptr(Ast const * ast, AstIdx idx) {
	uptr p = ast->pool.stack.end - (idx << 2);
	ASSERT_DEBUG(ast->pool.pos <= p && p <= ast->pool.end, "invalid [idx=%u]", idx);
	return (void*)p;
};

AstIdx ast_ptr_to_idx(CallLoc call, Ast const * ast, void const * ptr) {
	uptr p = (uptr)ptr;
	ASSERT_DEBUG_AT(call, p % AST_NODE_ALIGN == 0, "invalid [ptr=%p]", ptr);
	ASSERT_DEBUG_AT(call, ast->pool.pos <= p && p <= ast->pool.end, "invalid [ptr=%p]", ptr);
	usz  i = ((ast->pool.end - p) / AST_NODE_ALIGN);
	ASSERT_DEBUG_AT(call, i < AST_IDX_INVALID, "index oveflow");
	return (AstIdx)i;
};

#define DEPTH_STEP 2

void ast_fmt_ex(
	FmtVirt * fmt,
	Ast const * ast,
	AstIdx idx,
	u16 depth,
	u16 num, i16 width
) {
	AstNode const * head = AST_IDX_TO_PTR(ast, idx);

	fmt_write(fmt,
		FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)),
		FMT_COLOR(FMT_BLUE), FMT_LIT("[N"), FMT_U64(num, .width = width, .num = FMT_NUM_FILL), FMT_LIT("] ")
	);

	switch (head->kind) {
		case AST_K_POISON: {
			fmt_write(fmt, FMT_COLOR(FMT_RED), FMT_LIT("poison"), FMT_LINE);
		} break;

		case AST_K_ATOM_IDT:
		case AST_K_ATOM_NUM:
		case AST_K_ATOM_STR: {
			AstAtom const * atom = (void*)head;

			Str name; switch (head->kind) {
				case AST_K_ATOM_IDT: name = STR("ident"); break;
				case AST_K_ATOM_NUM: name = STR("number"); break;
				case AST_K_ATOM_STR: name = STR("string"); break;
				default: PANIC("unhandled atom kind (%u)", head->kind);
			};

			usz const pos = AST_NODE_POS(&atom->node_pos);
			Str lex = str_sub(ast->src.str_z, pos, pos + atom->len);
			fmt_write(fmt,
				FMT_COLOR(FMT_GREY), FMT_STR(name), FMT_LIT(" "),
				FMT_COLOR(FMT_GREEN), FMT_LIT("`"), FMT_STR(lex), FMT_LIT("`"),
				FMT_LINE
			);
		} break;

		case AST_K_UNA_POS:
		case AST_K_UNA_NEG:
		case AST_K_UNA_NOT:
		case AST_K_UNA_DOT:
		case AST_K_UNA_REF:
		case AST_K_UNA_DEREF:
		case AST_K_UNA_IF:
		{
			AstUna const * una = (void*)head;

			usz len;
			switch (head->kind) {
				case AST_K_UNA_POS:   len = 1; break;
				case AST_K_UNA_NEG:   len = 1; break;
				case AST_K_UNA_NOT:   len = 1; break;
				case AST_K_UNA_DOT:   len = 1; break;
				case AST_K_UNA_REF:   len = 1; break;
				case AST_K_UNA_DEREF: len = 2; break;
				case AST_K_UNA_IF:    len = 2; break;
				default: PANIC("unhandled atom kind (%u)", head->kind);
			};

			usz const pos = AST_NODE_POS(&una->node_pos);
			Str lex = str_sub(ast->src.str_z, pos, pos + len);
			fmt_write(fmt,
				FMT_COLOR(FMT_ORANGE), FMT_LIT("`"), FMT_STR(lex), FMT_LIT("` "),
				FMT_COLOR(FMT_GREY), FMT_LIT("("), FMT_LINE,
			);
			ast_fmt_ex(fmt, ast, AST_PTR_TO_IDX(ast, &una->n), depth + 1, 0, 1);
			fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
			fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LIT(")"), FMT_LINE);
		} break;

		case AST_K_BIN_ADD:
		case AST_K_BIN_SUB:
		case AST_K_BIN_MUL:
		case AST_K_BIN_DIV:

		case AST_K_BIN_DOT:
		case AST_K_BIN_TICK:

		case AST_K_BIN_COLON:
		case AST_K_BIN_SET:
		case AST_K_BIN_DEF:

		case AST_K_BIN_ARROW:
		case AST_K_BIN_IMPLIES:
		case AST_K_BIN_EXTEND:
		case AST_K_BIN_APPLY:

		case AST_K_BIN_THEN:
		case AST_K_BIN_ELIF:
		case AST_K_BIN_ELSE:
		{
			AstBin const * bin = (void*)head;

			usz len;
			switch (head->kind) {
				case AST_K_BIN_ADD:    len = 1; break;
				case AST_K_BIN_SUB:    len = 1; break;
				case AST_K_BIN_MUL:    len = 1; break;
				case AST_K_BIN_DIV:    len = 1; break;

				case AST_K_BIN_DOT:    len = 1; break;
				case AST_K_BIN_TICK:   len = 1; break;

				case AST_K_BIN_COLON:  len = 1; break;
				case AST_K_BIN_SET:    len = 1; break;
				case AST_K_BIN_DEF:    len = 2; break;

				case AST_K_BIN_ARROW:   len = 2; break;
				case AST_K_BIN_IMPLIES: len = 2; break;
				case AST_K_BIN_EXTEND:  len = 2; break;
				case AST_K_BIN_APPLY:   len = 2; break;

				case AST_K_BIN_THEN:   len = 4; break;
				case AST_K_BIN_ELIF:   len = 4; break;
				case AST_K_BIN_ELSE:   len = 4; break;
				default: PANIC("unhandled atom kind (%u)", head->kind);
			};

			usz const pos = AST_NODE_POS(&bin->node_pos);
			Str lex = str_sub(ast->src.str_z, pos, pos + len);
			fmt_write(fmt,
				FMT_COLOR(FMT_ORANGE), FMT_LIT("`"), FMT_STR(lex), FMT_LIT("` "),
				FMT_COLOR(FMT_GREY), FMT_LIT("("), FMT_LINE,
			);

			ast_fmt_ex(fmt, ast, bin->lhs, depth + 1, 0, 1);
			ast_fmt_ex(fmt, ast, AST_PTR_TO_IDX(ast, &bin->rhs), depth + 1, 1, 1);

			fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
			fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LIT(")"), FMT_LINE);
		} break;

		case AST_K_SEQ: {
			AstSeq const * seq = (void*)head;
			i16 const w = (head->with.seq.count == 0) ? 0 : u64_len(head->with.seq.count - 1, 10);

			Str lex; switch (AST_SEQ_DELIM(seq)) {
				case AST_D_NONE:  lex = STR("seq.none"); break;
				case AST_D_COMMA: lex = STR("seq.comma"); break;
				case AST_D_SEMI:  lex = STR("seq.semi"); break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(seq));
			};

			Str open; Str close; switch (AST_SEQ_BOUND(seq)) {
				case AST_B_EOF:     open = STR("<("); close = STR(")>"); break;
				case AST_B_PAREN:   open =  STR("("); close = STR(")"); break;
				case AST_B_BRACE:   open =  STR("{"); close = STR("}"); break;
				case AST_B_BRACKET: open =  STR("[N"); close = STR("]"); break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(seq));
			};

			fmt_write(fmt,
				FMT_COLOR(FMT_BLUE), FMT_STR(lex),
				// FMT_COLOR(FMT_RED), FMT_U64(seq->node_seq.delim_and_bound, .width = 10, .num = FMT_NUM_BIN | FMT_NUM_FILL),
				FMT_COLOR(FMT_GREY), FMT_LIT(" "), FMT_STR(open),
			);

			if (seq->node_seq.count != 0) {
				fmt_write(fmt, FMT_LINE);

				for (u16 i = 0;; i += 1) {
					if (seq->node_seq.count == 0 || i >= seq->node_seq.count - 1) break;
					ast_fmt_ex(fmt, ast, seq->exprs[i], depth + 1, i, w);
					// fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LINE);
				};

				if (seq->node_seq.count >= 1) {
					u16 const i = seq->node_seq.count - 1;
					ast_fmt_ex(fmt, ast, seq->exprs[i], depth + 1, i, w);
				};

				fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
			};

			fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_STR(close), FMT_LINE);
		} break;

		case AST_K_CALL_POSTFIX: {
			AstCall const * call = (void*)head;
			i16 const w = (head->with.seq.count == 0) ? 0 : u64_len(head->with.seq.count - 1, 10);

			Str lex; switch (AST_SEQ_DELIM(call)) {
				case AST_D_NONE:  lex = STR("call-postfix:none"); break;
				case AST_D_COMMA: lex = STR("call-postfix:comma"); break;
				case AST_D_SEMI:  lex = STR("call-postfix:semi");  break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(call));
			};

			Str open; Str close; switch (AST_SEQ_BOUND(call)) {
				case AST_B_EOF:     open = STR("<{"); close = STR("}>"); break;
				case AST_B_PAREN:   open = STR("(");  close = STR(")"); break;
				case AST_B_BRACE:   open = STR("{");  close = STR("}"); break;
				case AST_B_BRACKET: open = STR("[N");  close = STR("]"); break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(call));
			};

			fmt_write(fmt,
				FMT_COLOR(FMT_ORANGE), FMT_STR(lex),
				// FMT_COLOR(FMT_RED), FMT_U64(seq->node_seq.delim_and_bound, .width = 10, .num = FMT_NUM_BIN | FMT_NUM_FILL),
				FMT_COLOR(FMT_GREY), FMT_LIT(" "), FMT_STR(open), FMT_LINE,
			);

			ast_fmt_ex(fmt, ast, call->fun, depth + 1, 0, w);

			for (u16 i = 0;; i += 1) {
				if (call->node_seq.count == 0 || i >= call->node_seq.count - 1) break;
				ast_fmt_ex(fmt, ast, call->exprs[i], depth + 1, i + 1, w);
				// fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LINE);
			};

			if (call->node_seq.count >= 1) {
				u16 const i = call->node_seq.count - 1;
				ast_fmt_ex(fmt, ast, call->exprs[i], depth + 1, i + 1, w);
			};

			fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
			fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_STR(close), FMT_LINE);
		} break;

		case AST_K_ROOT: {
			AstRoot const * seq = (void*)head;
			i16 const w = (head->with.seq.count == 0) ? 0 : u64_len(head->with.seq.count - 1, 10);

			Str lex; switch (AST_SEQ_DELIM(seq)) {
				case AST_D_NONE:  lex = STR("root.none"); break;
				case AST_D_COMMA: lex = STR("root.comma"); break;
				case AST_D_SEMI:  lex = STR("root.semi"); break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(seq));
			};

			Str open; Str close; switch (AST_SEQ_BOUND(seq)) {
				case AST_B_EOF:     open = STR("<("); close = STR(")>"); break;
				case AST_B_PAREN:   open =  STR("("); close = STR(")"); break;
				case AST_B_BRACE:   open =  STR("{"); close = STR("}"); break;
				case AST_B_BRACKET: open =  STR("[N"); close = STR("]"); break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(seq));
			};

			fmt_write(fmt,
				FMT_COLOR(FMT_BLUE), FMT_STR(lex),
				// FMT_COLOR(FMT_RED), FMT_U64(seq->node_seq.delim_and_bound, .width = 10, .num = FMT_NUM_BIN | FMT_NUM_FILL),
				FMT_COLOR(FMT_GREY), FMT_LIT(" "), FMT_STR(open), FMT_LINE,
			);

			for (u16 i = 0;; i += 1) {
				if (seq->node_seq.count == 0 || i >= seq->node_seq.count - 1) break;
				ast_fmt_ex(fmt, ast, seq->exprs[i], depth + 1, i, w);
				// fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_LINE);
			};

			if (seq->node_seq.count >= 1) {
				u16 const i = seq->node_seq.count - 1;
				ast_fmt_ex(fmt, ast, seq->exprs[i], depth + 1, i, w);
			};

			fmt_write(fmt, FMT_LIT("", .width = (i16)(-depth * DEPTH_STEP)));
			fmt_write(fmt, FMT_COLOR(FMT_GREY), FMT_STR(close), FMT_LINE);
	
		} break;
		default: TODO("unhandled kind (%s)", ast_name(head->kind).ptr);
	};
};

void ast_fmt(FmtVirt * fmt, Ast const * ast, AstIdx idx) {
	ast_fmt_ex(fmt, ast, idx, 0, 0, 1);
	fmt_write(fmt, FMT_LINE);
};

void ast_graphviz_recursive(FmtVirt * fmt, Ast const * ast, AstIdx idx) {
	AstNode const * head = AST_IDX_TO_PTR(ast, idx);

	switch (head->kind) {
		case AST_K_POISON: {
			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx),
				FMT_LIT("] POISON |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT(" \\l|" "\n"
					"\t" "\t" "\t" "\t" ":pos = "), FMT_U64(AST_NODE_POS(&head->with.pos)),
				FMT_LIT("\\l" "\n"
					"\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n" "\n"
				),
			);
		} break;

		case AST_K_ATOM_IDT:
		case AST_K_ATOM_NUM:
		case AST_K_ATOM_STR: {
			AstAtom const * atom = (void*)head;

			usz const pos = AST_NODE_POS(&atom->node_pos);
			Str lex = str_sub(ast->src.str_z, pos, pos + atom->len);

			if (lex.len != 0 && lex.ptr[0] == '"') {
				lex.ptr += 1;
				lex.len -= 1;
			};

			if (lex.len != 0 && lex.ptr[lex.len - 1] == '"') {
				lex.len -= 1;
			};

			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx), FMT_LIT("] `"), FMT_STR(lex), FMT_LIT("`"),
				FMT_LIT(" |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT(" \\l|" "\n"
					"\t" "\t" "\t" "\t" ":pos = "), FMT_U64(pos),
				FMT_LIT("\\l" "\n" "\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n" "\n"
				),
			);
		} break;

		case AST_K_UNA_POS:
		case AST_K_UNA_NEG:
		case AST_K_UNA_NOT:
		case AST_K_UNA_DOT:
		case AST_K_UNA_REF:
		case AST_K_UNA_DEREF:
		case AST_K_UNA_IF:
		{
			AstUna const * una = (void*)head;

			usz len;
			switch (head->kind) {
				case AST_K_UNA_POS:   len = 1; break;
				case AST_K_UNA_NEG:   len = 1; break;
				case AST_K_UNA_NOT:   len = 1; break;
				case AST_K_UNA_DOT:   len = 1; break;
				case AST_K_UNA_REF:   len = 1; break;
				case AST_K_UNA_DEREF: len = 2; break;
				case AST_K_UNA_IF:    len = 2; break;
				default: PANIC("unhandled atom kind (%u)", head->kind);
			};

			usz const pos = AST_NODE_POS(&una->node_pos);
			Str lex = str_sub(ast->src.str_z, pos, pos + len);

			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx), FMT_LIT("] `"), FMT_STR(lex), FMT_LIT("`"),
				FMT_LIT(" |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT(" \\l|" "\n"
					"\t" "\t" "\t" "\t" ":pos = "), FMT_U64(pos),
				FMT_LIT("\\l" "\n" "\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n"
				),
			);

			AstIdx const node = AST_PTR_TO_IDX(ast, &una->n);

			fmt_write(fmt,
				FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
				FMT_LIT(" -> "),
				FMT_LIT("N"), FMT_U64(node),
				FMT_LIT(";" "\n" "\n")
			);

			ast_graphviz_recursive(fmt, ast, node);
		} break;

		case AST_K_BIN_ADD:
		case AST_K_BIN_SUB:
		case AST_K_BIN_MUL:
		case AST_K_BIN_DIV:

		case AST_K_BIN_DOT:
		case AST_K_BIN_TICK:

		case AST_K_BIN_COLON:
		case AST_K_BIN_SET:
		case AST_K_BIN_DEF:

		case AST_K_BIN_ARROW:
		case AST_K_BIN_IMPLIES:
		case AST_K_BIN_EXTEND:
		case AST_K_BIN_APPLY:

		case AST_K_BIN_LT:
		case AST_K_BIN_LE:
		case AST_K_BIN_GT:
		case AST_K_BIN_GE:

		case AST_K_BIN_EQ:
		case AST_K_BIN_NEQ:

		case AST_K_BIN_LOG_OR:
		case AST_K_BIN_LOG_AND:

		case AST_K_BIN_THEN:
		case AST_K_BIN_ELIF:
		case AST_K_BIN_ELSE:
		{
			AstBin const * bin = (void*)head;

			// usz len;
			// switch (head->kind) {
			// 	case AST_K_BIN_ADD:    len = 1; break;
			// 	case AST_K_BIN_SUB:    len = 1; break;
			// 	case AST_K_BIN_MUL:    len = 1; break;
			// 	case AST_K_BIN_DIV:    len = 1; break;
			//
			// 	case AST_K_BIN_DOT:    len = 1; break;
			// 	case AST_K_BIN_TICK:   len = 1; break;
			//
			// 	case AST_K_BIN_COLON:  len = 1; break;
			// 	case AST_K_BIN_SET:    len = 1; break;
			// 	case AST_K_BIN_DEF:    len = 2; break;
			//
			// 	case AST_K_BIN_ARROW:   len = 2; break;
			// 	case AST_K_BIN_IMPLIES: len = 2; break;
			// 	case AST_K_BIN_EXTEND:  len = 2; break;
			// 	case AST_K_BIN_APPLY:   len = 2; break;
			//
			// 	case AST_K_BIN_THEN:   len = 4; break;
			// 	case AST_K_BIN_ELIF:   len = 4; break;
			// 	case AST_K_BIN_ELSE:   len = 4; break;
			// 	default: PANIC("unhandled atom kind (%u)", head->kind);
			// };
			//
			usz const pos = AST_NODE_POS(&bin->node_pos);
			// Str lex = str_sub(ast->src.str_z, pos, pos + len);

			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx),
				FMT_LIT("] `"), FMT_STR(ast_tag(head->kind)), FMT_LIT("`"),
				FMT_LIT(" |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT(" \\l|" "\n"
					"\t" "\t" "\t" "\t" ":pos = "), FMT_U64(pos),
				FMT_LIT("\\l" "\n" "\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n"
				),
			);

			fmt_write(fmt,
				FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
				FMT_LIT(" -> "),
				FMT_LIT("N"), FMT_U64(bin->lhs),
				FMT_LIT(";" "\n")
			);

			fmt_write(fmt,
				FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
				FMT_LIT(" -> "),
				FMT_LIT("N"), FMT_U64(AST_PTR_TO_IDX(ast, &bin->rhs)),
				FMT_LIT(";" "\n" "\n")
			);

			ast_graphviz_recursive(fmt, ast, bin->lhs);
			ast_graphviz_recursive(fmt, ast, AST_PTR_TO_IDX(ast, &bin->rhs));
		} break;

		case AST_K_SEQ: {
			AstSeq const * seq = (void*)head;
			// i16 const w = (head->with.seq.count == 0) ? 0 : u64_len(head->with.seq.count - 1, 10);

			Str lex; switch (AST_SEQ_DELIM(seq)) {
				case AST_D_NONE:  lex = STR("seq.none"); break;
				case AST_D_COMMA: lex = STR("seq.comma"); break;
				case AST_D_SEMI:  lex = STR("seq.semi"); break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(seq));
			};

			// Str open; Str close; switch (AST_SEQ_BOUND(seq)) {
			// 	case AST_B_EOF:     open = STR("<("); close = STR(")>"); break;
			// 	case AST_B_PAREN:   open =  STR("("); close = STR(")"); break;
			// 	case AST_B_BRACE:   open =  STR("{"); close = STR("}"); break;
			// 	case AST_B_BRACKET: open =  STR("[N"); close = STR("]"); break;
			// 	default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(seq));
			// };

			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx), FMT_LIT("] `"), FMT_STR(lex), FMT_LIT("`"),
				FMT_LIT(" |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT("\\l")
			);

			for (u16 i = 0; i < seq->node_seq.count; i += 1) {
				fmt_write(fmt,
					FMT_LIT(" |" "\n" "\t" "\t" "\t" "<N"), FMT_U64(seq->exprs[i]),
					FMT_LIT("> "),
					FMT_LIT("N"), FMT_U64(seq->exprs[i]),
				);
			};

			fmt_write(fmt,
				FMT_LIT("\n" "\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n"
				),
			);
			
			for (u16 i = 0; i < seq->node_seq.count; i += 1) {
				fmt_write(fmt,
					FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
					FMT_LIT(":N"), FMT_U64(seq->exprs[i]),
					FMT_LIT(" -> "),
					FMT_LIT("N"), FMT_U64(seq->exprs[i]),
					FMT_LIT(";\n"),
				);
			};

			fmt_write(fmt, FMT_LINE);

			for (u16 i = 0; i < seq->node_seq.count; i += 1) {
				ast_graphviz_recursive(fmt, ast, seq->exprs[i]);
			};

		} break;

		case AST_K_CALL_POSTFIX: {
			AstCall const * call = (void*)head;

			Str lex; switch (AST_SEQ_DELIM(call)) {
				case AST_D_NONE:  lex = STR("call-postfix:none"); break;
				case AST_D_COMMA: lex = STR("call-postfix:comma"); break;
				case AST_D_SEMI:  lex = STR("call-postfix:semi");  break;
				default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(call));
			};

			// Str open; Str close; switch (AST_SEQ_BOUND(call)) {
			// 	case AST_B_EOF:     open = STR("<{"); close = STR("}>"); break;
			// 	case AST_B_PAREN:   open = STR("(");  close = STR(")"); break;
			// 	case AST_B_BRACE:   open = STR("{");  close = STR("}"); break;
			// 	case AST_B_BRACKET: open = STR("[N");  close = STR("]"); break;
			// 	default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(call));
			// };

			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx), FMT_LIT("] `"), FMT_STR(lex), FMT_LIT("`"),
				FMT_LIT(" |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT("\\l"),
			);

			fmt_write(fmt,
					FMT_LIT(" |" "\n" "\t" "\t" "\t" "<N"), FMT_U64(call->fun),
					FMT_LIT("> "),
					FMT_LIT("N"), FMT_U64(call->fun),
			);

			for (u16 i = 0; i < call->node_seq.count; i += 1) {
				fmt_write(fmt,
					FMT_LIT(" |" "\n" "\t" "\t" "\t" "<N"), FMT_U64(call->exprs[i]),
					FMT_LIT("> "),
					FMT_LIT("N"), FMT_U64(call->exprs[i]),
				);
			};

			fmt_write(fmt,
				FMT_LIT("\n" "\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n"
				),
			);
			
			fmt_write(fmt,
				FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
				FMT_LIT(":N"), FMT_U64(call->fun),
				FMT_LIT(" -> "),
				FMT_LIT("N"), FMT_U64(call->fun),
				FMT_LIT(";\n"),
			);

			for (u16 i = 0; i < call->node_seq.count; i += 1) {
				fmt_write(fmt,
					FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
					FMT_LIT(":N"), FMT_U64(call->exprs[i]),
					FMT_LIT(" -> "),
					FMT_LIT("N"), FMT_U64(call->exprs[i]),
					FMT_LIT(";\n"),
				);
			};

			fmt_write(fmt, FMT_LINE);

			ast_graphviz_recursive(fmt, ast, call->fun);
			for (u16 i = 0; i < call->node_seq.count; i += 1) {
				ast_graphviz_recursive(fmt, ast, call->exprs[i]);
			};
		} break;

		case AST_K_ROOT: {
			AstRoot const * seq = (void*)head;

			// Str lex; switch (AST_SEQ_DELIM(seq)) {
				// case AST_D_NONE:  lex = STR("root.none"); break;
				// case AST_D_COMMA: lex = STR("root.comma"); break;
				// case AST_D_SEMI:  lex = STR("root.semi"); break;
				// default: PANIC("unhandled kind (%u)", AST_SEQ_DELIM(seq));
			// };

			// Str open; Str close; switch (AST_SEQ_BOUND(seq)) {
			// 	case AST_B_EOF:     open = STR("<("); close = STR(")>"); break;
			// 	case AST_B_PAREN:   open =  STR("("); close = STR(")"); break;
			// 	case AST_B_BRACE:   open =  STR("{"); close = STR("}"); break;
			// 	case AST_B_BRACKET: open =  STR("[N"); close = STR("]"); break;
			// 	default: PANIC("unhandled kind (%u)", AST_SEQ_BOUND(seq));
			// };

			fmt_write(fmt,
				FMT_LIT("\t\t" "N"), FMT_U64(idx),
				FMT_LIT(
					" [" "\n"
					"\t" "\t" "\t" "label=\"{" "\n"
					"\t" "\t" "\t" "\t" "[N"), FMT_U64(idx),
				FMT_LIT("] `"), FMT_STR(ast_tag(head->kind)), FMT_LIT("`"),
				FMT_LIT(" |" "\n"
					"\t" "\t" "\t" "\t" ":tag = "), FMT_STR(ast_tag(head->kind)),
				FMT_LIT("\\l"), 
			);

			for (u16 i = 0; i < seq->node_seq.count; i += 1) {
				fmt_write(fmt,
					FMT_LIT(" |" "\n" "\t" "\t" "\t" "<N"), FMT_U64(seq->exprs[i]),
					FMT_LIT("> "),
					FMT_LIT("N"), FMT_U64(seq->exprs[i]),
				);
			};

			fmt_write(fmt,
				FMT_LIT("\n" "\t" "\t" "\t" "}\"" "\n"
					"\t" "\t" "];" "\n"
				),
			);
			
			for (u16 i = 0; i < seq->node_seq.count; i += 1) {
				fmt_write(fmt,
					FMT_LIT("\t\t\t" "N"), FMT_U64(idx),
					FMT_LIT(":N"), FMT_U64(seq->exprs[i]),
					FMT_LIT(" -> "),
					FMT_LIT("N"), FMT_U64(seq->exprs[i]),
					FMT_LIT(";\n"),
				);
			};

			fmt_write(fmt, FMT_LINE);

			for (u16 i = 0; i < seq->node_seq.count; i += 1) {
				ast_graphviz_recursive(fmt, ast, seq->exprs[i]);
			};

		} break;
		default: TODO("unhandled kind (%s)", ast_name(head->kind).ptr);
	};
};

void ast_graphviz(FmtVirt * fmt, Ast const * ast, AstIdx idx) {
	fmt_write(fmt, FMT_LIT(
		"digraph {" "\n"
		"\t" "bgcolor=\"#1f1f28\"" "\n"
		"\t" "rankdir=TB;" "\n"
		"\t" "ranksep=0.8;" "\n"
		"\t" "nodesep=0.4;" "\n"
		"\t" "ordering=out;" "\n"
		"\t" "splines=true;" "\n"
		"\n"
		"\t" "subgraph ast {" "\n"
		"\t" "\t" "label=\"AST\"" "\n"
		"\n"
		"\t" "\t" "node [" "\n"
		"\t" "\t" "\t" "shape=record" "\n"
		"\t" "\t" "\t" "style=\"rounded,filled\"" "\n"
		"\n"
		"\t" "\t" "\t" "fontname=\"JetBrains Mono\"" "\n"
		"\t" "\t" "\t" "fontsize=12" "\n"
		"\n"
		"\t" "\t" "\t" "color=\"#7e9cd8\"" "\n"
		"\t" "\t" "\t" "fontcolor=\"#dcd7ba\"" "\n"
		"\t" "\t" "\t" "fillcolor=\"#2a2a37\"" "\n"
		"\t" "\t" "\t" "penwidth=2" "\n"
		"\t" "\t" "];" "\n"
		"\n"
		"\t" "\t" "edge [" "\n"
		"\t" "\t" "\t" "color=\"#727169\"" "\n"
		"\t" "\t" "\t" "penwidth=1.5" "\n"
		"\t" "\t" "\t" "arrowsize=0.8" "\n"
		"\t" "\t" "];" "\n"
		"\n"
	));

	ast_graphviz_recursive(fmt, ast, idx);

	fmt_write(fmt, FMT_LIT(
		"\t" "}" "\n"
		"}" "\n"
	));
};

