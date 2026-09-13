#include "trc/assert.h"
#include "fmt/fmt.h"
#include "src/src.h"
#include "lex/stream.h"

typedef struct {
	Src src; FmtVirt * out;
	Lexer lexer;
	LexTokenVec tokens;
} LexState;

#define RK_DEBUG_LOCATION_ENABLE

#define lex_string_unterminated(state, pos) lex_string_unterminated_ex(CALL, state, pos)

static bool lex_string_unterminated_ex(CallLoc call, LexState * state, usz pos) {
	SrcDot dot = src_eval_dot(state->src.str_z, pos);
	return fmt_virt_write(state->out,
		// main message
		FMT_COLOR(FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("unterminated string"), FMT_LIT("\n"),

		// location
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC(state->src.path.str_z, dot.row, dot.col),

#ifdef RK_DEBUG_LOCATION_ENABLE
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
#endif
		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
	).ptr != NULL;
};

#define lex_quotes_unterminated(state, pos) lex_quotes_unterminated_ex(CALL, state, pos)

static bool lex_quotes_unterminated_ex(CallLoc call, LexState * state, usz pos) {
	SrcDot dot = src_eval_dot(state->src.str_z, pos);
	return fmt_virt_write(state->out,
		// main message
		FMT_COLOR(FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("unterminated string in quotes '`...', use '\"...\"' instead"), FMT_LIT("\n"),

		// location
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC(state->src.path.str_z, dot.row, dot.col),

#ifdef RK_DEBUG_LOCATION_ENABLE
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
#endif
		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
	).ptr != NULL;
};

#define lex_typo(state, atom) lex_typo_ex(CALL, state, atom)

static LexKindTypo lex_typo_ex(CallLoc call, LexState * state, LexToken atom) {
	ASSERT_DEBUG(LEX_IS_TYPO(atom.kind), "expected LEX_TYPO kind");
	SrcDot dot = src_eval_dot(state->src.str_z, atom.pos);

	Str typo = (Str){.ptr = &state->src.str_z.ptr[atom.pos], .len = lex_len(atom)};
	Str valid;
	LexKindTypo kind;

	switch (atom.kind) {
		case LEX_TYPO_SLASH_COMMENT: {
			typo  = STR("// ...");
			valid = STR("# ...");
			kind  = LEX_EXT_COMMENT;
			break;
		};
		case LEX_TYPO_DOLLAR:     valid = STR("money");   kind = LEX_TYPO_KIND_LAST; break;
		case LEX_TYPO_QUOTES:     valid = STR("\"...\""); kind = LEX_TYPO_KIND_LAST; break;
		case LEX_TYPO_BACK_SLASH: valid = STR("\\>");     kind = LEX_ILLEGAL;        break;
		default: PANIC("not handled kind");
	};

	ASSERT(fmt_virt_write(state->out,
		// main message
		FMT_COLOR(FMT_RED), FMT_LIT("[risk:error] "), FMT_COLOR(FMT_WHITE), FMT_LIT("typo "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(typo), FMT_LIT("'"),
		FMT_COLOR(FMT_WHITE), FMT_LIT(", maybe you mean "),
		FMT_COLOR(FMT_GREEN), FMT_LIT("'"), FMT_STR(valid), FMT_LIT("'"),
		FMT_LIT("\n"),

		// location
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC(state->src.path.str_z, dot.row, dot.col),

#ifdef RK_DEBUG_LOCATION_ENABLE
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
#endif

		// end
		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
	).ptr != NULL, "");

	return kind;
};

Lex lex_analyze(Memory * mem, FmtVirt * out, Src src) {
	LexState state = {
		.src = src, .out = out,
		.lexer = lex_init(src.str_z),
		.tokens.virt_vec = virt_vec_init(LexTokenVec, mem, .size = MB(128)),
	};

	for (;;) {
		LexToken token = lex_next(&state.lexer);

		if (token.kind == LEX_STRING_UNTERMINATED) {
			lex_string_unterminated(&state, token.pos);
		} else if (token.kind == LEX_TYPO_QUOTES_UNTERMINATED) {
			lex_quotes_unterminated(&state, token.pos);
			token.kind = LEX_STRING_UNTERMINATED;
		} else if (LEX_IS_TYPO(token.kind)) {
			token.kind = lex_typo(&state, token);
		};

		if (token.kind >= LEX_KIND_LAST) continue;

		// Str const kind = lex_token_name(token.kind);
		// Str const lex = {.ptr = &src.str_z.ptr[token.pos], .len = lex_len(token)};
		// fmt_write(state.out,
		// 	FMT_COLOR(FMT_ORANGE), FMT_STR(kind, .width = -14),
		// 	FMT_COLOR(FMT_WHITE), FMT_LIT(" | "),
		// 	FMT_COLOR(FMT_GREEN), FMT_LIT("`"), FMT_STR(lex), FMT_LIT("`"), FMT_LINE,
		// );

		ASSERT(VIRT_VEC_PUSH(&state.tokens, token), "");
		if (token.kind == LEX_EOF) break;
	};

	f64 const ratio = (f64)src.len / (f64)state.tokens.len;

	fmt_write(
		state.out,
		FMT_COLOR(FMT_BLUE), FMT_LIT("[info] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("collected "),
		FMT_COLOR(FMT_BLUE), FMT_U64(state.tokens.len), FMT_LIT(" tokens"),
		FMT_COLOR(FMT_PURPLE),
		FMT_LIT(" (ratio: "),
		FMT_F64(ratio, .prec = 1),
		FMT_LIT(") "), FMT_LOC_DEBUG(CALL),
	);

	return (Lex){.src = src, .tokens = state.tokens};
};

// #define lex_unknown_escape(state, pos, atom) lex_unknown_escape_ex(CALL, state, pos, atom)
//
// static bool lex_unknown_escape_ex(CallLoc call, LexState * state, usz pos, LexToken atom) {
// 	Str const escape = str_cut(state->src.str_z, pos, pos + 2);
// 	// UNUSED(atom);
// 	Str const string = str_cut(state->src.str_z, atom.pos, atom.pos + atom.len);
// 	u16 const offset = (u16)(STR("literal: ").len + (pos - atom.pos));
//
// 	SrcDot const dot = src_eval_dot(state->src.str_z, pos);
// 	return fmt_write(state->out,
// 		// main message
// 		FMT_COLOR(FMT_RED), FMT_LIT("error: "),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("unknown escape sequence "),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(escape)), FMT_LIT("\n", FMT_LIT("'"),
// 		// location
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC(state->src.path.str_z, dot.row, dot.col),
//
// 		// view
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_COLOR(FMT_WHITE), FMT_LIT("literal: "),
// 		FMT_COLOR(FMT_GREEN), FMT_STR(string), FMT_LIT("\n"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LIT(" ", .repeat = offset),
// 		FMT_LIT("^^ here "), FMT_LIT("'"), FMT_STR(escape)), FMT_LIT("\n", FMT_LIT("'"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// 		// note
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_COLOR(FMT_BLUE), FMT_LIT("note: "),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("there are only "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'\\\\'"), FMT_COLOR(FMT_WHITE), FMT_LIT(", "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'\\\"'"), FMT_COLOR(FMT_WHITE), FMT_LIT(", "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'\\t'"),  FMT_COLOR(FMT_WHITE), FMT_LIT(", "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'\\r'"),  FMT_COLOR(FMT_WHITE), FMT_LIT(", "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'\\n'"),  FMT_COLOR(FMT_WHITE), FMT_LIT(" and "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'\\0'"),             FMT_LIT("\n"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// #ifdef RK_DEBUG_LOCATION_ENABLE
// 		// debug
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
// #endif
// 		FMT_DONE,
// 	);
// };

// #define lex_invalid_char_in_number(state, pos, atom) lex_invalid_char_in_number_ex(CALL, state, pos, atom)
//
// static bool lex_invalid_char_in_number_ex(CallLoc call, LexState * state, usz pos, LexToken atom) {
// 	SrcDot dot = src_eval_dot(state->src.str_z, pos);
// 	Str const byte = (Str){.ptr = &state->src.ptr_z[pos], .len = 1};
// 	Str const number = (Str){.ptr = &state->src.ptr_z[atom.pos], .len = atom.len};
// 	u16 const offset = (u16)(STR("literal: '").len + (pos - atom.pos));
// 	return fmt_write(state->out,
// 		// main message
// 		FMT_COLOR(FMT_RED), FMT_LIT("error: "),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("invalid character "),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(byte), FMT_LIT("'"),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT(" in number "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'"), FMT_STR(number), FMT_LIT("'"),
// 		FMT_LIT("\n"),
//
// 		// location
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC(state->src.path.str_z, dot.row, dot.col),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// 		// view
// 		FMT_COLOR(FMT_RED), FMT_LIT("|."), FMT_COLOR(FMT_WHITE), FMT_LIT("literal: "),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'"), FMT_STR(number)), FMT_LIT("\n", FMT_LIT("'"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LIT(" ", .repeat = offset),
// 		FMT_LIT("^ here "), FMT_LIT("'"), FMT_STR(byte)), FMT_LIT("\n", FMT_LIT("'"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// 		// note
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_COLOR(FMT_BLUE), FMT_LIT("@note: "),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("only decimal notation is available for numbers"), FMT_LIT("\n"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// #ifdef RK_DEBUG_LOCATION_ENABLE
// 		// debug
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
// #endif
//
// 		// end
// 		FMT_DONE,
// 	);
// };

// #define lex_number_literal_too_large(state, atom) lex_number_literal_too_large_ex(CALL, state, atom)
//
// static bool lex_number_literal_too_large_ex(CallLoc call, LexState * state, LexToken atom) {
// 	Str const number = (Str){.ptr = &state->src.ptr_z[atom.pos], .len = atom.len};
// 	SrcDot dot = src_eval_dot(state->src.str_z, atom.pos);
// 	u16 const repeat = (u16)(number.len - u64_decimal_len(U64_MAX));
// 	return fmt_write(state->out,
// 		// main message
// 		FMT_COLOR(FMT_RED), FMT_LIT("error: "),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("number literal is too large"), FMT_LIT("\n"),
//
// 		// location
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC(state->src.path.str_z, dot.row, dot.col),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// 		// view
// 		FMT_COLOR(FMT_RED), FMT_LIT("|."),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("number: "),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(number)), FMT_LIT("\n", FMT_LIT("'"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("|."),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("max:    "), FMT_LIT(" ", .repeat = repeat),
// 		FMT_COLOR(FMT_BLUE), FMT_LIT("'"), FMT_U64(U64_MAX)), FMT_LIT("\n", FMT_LIT("'"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// 		// note
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_COLOR(FMT_BLUE), FMT_LIT("@note: "),
// 		FMT_COLOR(FMT_WHITE), FMT_LIT("number literals are currently limited to u64"), FMT_LIT("\n"),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
//
// #ifdef RK_DEBUG_LOCATION_ENABLE
// 		// debug
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
// 		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
// 		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
// #endif
//
// 		// end
// 		FMT_DONE,
// 	);
// };

// static LexIdent lexanalyze_string_line(LexState * state, LexToken atom, Str lex) {
// 	// allocate temp string
// 	u8 * ptr = virt_bump_array(&state->tmp, u8, atom.len);
// 	ASSERT(ptr != NULL && virt_bump_commit(&state->tmp), "OOM");
//
// 	// escapes check
// 	usz w = 0; usz r = 0;
// 	for (;;) {
// 		if (r >= lex.len) break;
//
// 		// take bytes without escapes
// 		if (lex.ptr[r] != '\\') {
// 			usz j = r;
// 			for (;;) { j += 1;
// 				if (j >= atom.len || lex.ptr[j] == '\\') break;
// 			};
// 			usz const len = j - r;
// 			memcpy(&ptr[w], &lex.ptr[r], len);
//
// 			w += len; r += len; continue;
// 		};
//
// 		r += 1;
// 		usz const offset = r;
//
// 		if (FALSE) {
// 		} else if (lex.ptr[r] == '0') {
// 			ptr[w] = '\0'; w += 1; r += 1;
// 		} else if (lex.ptr[r] == '"') {
// 			ptr[w] = '\"'; w += 1; r += 1;
// 		} else if (lex.ptr[r] == 't') {
// 			ptr[w] = '\t'; w += 1; r += 1;
// 		} else if (lex.ptr[r] == 'r') {
// 			ptr[w] = '\r'; w += 1; r += 1;
// 		} else if (lex.ptr[r] == 'n') {
// 			ptr[w] = '\n'; w += 1; r += 1;
// 		} else {
// 			ptr[w] = '\\';       w += 1;
// 			ptr[w] = lex.ptr[r]; w += 1; r += 1;
// 			ASSERT(lex_unknown_escape(state, atom.pos + offset, atom), "");
// 		};
// 	};
//
// 	LexIdent const id = lexident_add(&state->pool, (Str){.ptr = ptr, .len = r});
// 	virt_bump_reset(&state->tmp);
// 	return id;
// };

// LexKind lexanalyze_number(LexState * state, LexToken atom, Str lex) {
// 	u8 * z = lex.ptr;
// 	u64 num = 0;
// 	bool error = FALSE;
//
// 	for (usz i = 0; i < lex.len; i += 1) {
// 		if (z[i] == '\'' || z[i] == '_') continue;
// 		if (z[i] < '0' || '9' < z[i]) {
// 			lex_invalid_char_in_number(state, atom.pos + i, atom);
// 			error = TRUE;
// 			break;
// 		};
//
// 		bool overflow = FALSE;
// 		overflow |= MUL_OVERFLOW(num, 10, &num);
// 		overflow |= ADD_OVERFLOW(num, z[i] - '0', &num);
//
// 		if (overflow) {
// 			lex_number_literal_too_large(state, atom);
// 			error = TRUE;
// 			break;
// 		};
// 	};
//
// 	return error ? LEX_NUMBER : 0;
// };


