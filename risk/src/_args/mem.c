#include "args/args.h"

////////////////////////////////
// parse

ENUM(ArgsMemParseKind, u8) {
	ARGS_MEM_PARSE_OK,
	ARGS_MEM_PARSE_EMPTY,
	ARGS_MEM_PARSE_OVERFLOW,
	ARGS_MEM_PARSE_INVALID_DIGIT,
	ARGS_MEM_PARSE_INVALID_SCALE,
};

typedef struct {
	u32 at;
	ArgsMemParseKind kind;
} ArgsMemParse;

bool args_mem_parse(u64 * x, u8 const * z, ArgsMemParse * parse) {
	ASSERT_DEBUG(x != NULL, "expected [x]");
	ASSERT_DEBUG(z != NULL, "expected [z]");
	ASSERT_DEBUG(parse != NULL, "expected [parse]");

	if (*z == '\0') {
		parse->kind = ARGS_MEM_PARSE_EMPTY;
		return FALSE;
	};

	u64 a = 0; u32 i = 0;
	if (z[i] < '0' || '9' < z[i]) {
		parse->kind = ARGS_MEM_PARSE_INVALID_DIGIT; parse->at = i;
		return FALSE;
	};

	a += z[i] - '0';
	for (;;) {
		i += 1;
		if (z[i] == '\0') goto ok;
		if (z[i] == '\'' || z[i] == '_') continue;
		if (z[i] < '0' || '9' < z[i]) break;
		if (MUL_OVER(a, 10, &a) || ADD_OVER(a, z[i] - '0', &a)) goto overflow;
	};

	if (z[i] == 'B' && z[i + 1] == '\0') goto ok;

	// [_, _, ...]
	//  ^  ^ char or '\0'
	//  |
	//  * not '\0'
	if ( // monstrosity
		// KB, MB, GB - check last 'B' - '?B'
		(z[i + 1] != 'B' || (z[i + 1] == 'B' && z[i + 2] != '\0')) &&
		// KiB, MiB, GiB - check last 'iB' - '?iB'
		(z[i + 1] != 'i' || (z[i + 1] == 'i' && z[i + 2] != 'B') || (z[i + 1] == 'i' && z[i + 2] == 'B' && z[i + 3] != '\0'))
	) goto invalid_scale;

	u64 scale;
	switch (z[i]) {
		case 'K': scale = KB(1); break;
		case 'M': scale = MB(1); break;
		case 'G': scale = GB(1); break;
		default: goto invalid_scale;
	};

	if (MUL_OVER(a, scale, &a)) goto overflow;

ok:
	parse->kind = ARGS_MEM_PARSE_OK; *x = a;
	return TRUE;

invalid_scale:
	parse->kind = ARGS_MEM_PARSE_INVALID_SCALE; parse->at = i;
	return FALSE;

overflow:
	parse->kind = ARGS_MEM_PARSE_OVERFLOW; parse->at = i;
	return FALSE;
};

////////////////////////////////
// diag

#define args_mem_expected(state, mem) args_mem_expected_ex(CALL, state, mem)
bool args_mem_expected_ex(CallLoc call, ArgsState * state, ArgsMem const * mem) {
	return fmt_write(state->out,
		// message
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_PURPLE), FMT_LIT("'--mem-limit'"),
		FMT_COLOR(FMT_WHITE),  FMT_LIT(" expect memory size, for example default is "),
		FMT_COLOR(FMT_PURPLE), FMT_LIT("'--mem-limit "), FMT_MEM(mem->min), FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

#define args_mem_parse_fail(state, parse) args_mem_parse_fail_ex(CALL, state, parse)
bool args_mem_parse_fail_ex(CallLoc call, ArgsState * state, ArgsMemParse parse) {
	u32 const suffix_len = (u32)state->peek.len - parse.at;
	switch (parse.kind) {
		case ARGS_MEM_PARSE_OK: return TRUE;
		case ARGS_MEM_PARSE_EMPTY: return fmt_write(state->out,
			// message
			FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("memory size is empty, please, provide"), FMT_LIT("\n"),
			// debug
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
			FMT_COLOR(FMT_RESET),
		).ptr != NULL;
		case ARGS_MEM_PARSE_OVERFLOW: return fmt_write(state->out,
			// message
			FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("memory size overflow"), FMT_LIT("\n"),
			// literal
			FMT_COLOR(FMT_RED), FMT_LIT("|.literal: "),
			FMT_COLOR(FMT_GREEN), FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"), FMT_LIT("\n"),
			FMT_COLOR(FMT_RED), FMT_LIT("|          "),
			FMT_LIT("", .width = (i16)parse.at + 1), FMT_LIT("^ overflow here"), FMT_LIT("\n"),
			// end
			FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
			// debug
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
			FMT_COLOR(FMT_RESET),
		).ptr != NULL;
		case ARGS_MEM_PARSE_INVALID_DIGIT: return fmt_write(state->out,
			// message
			FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("invalid digit in memory size"), FMT_LIT("\n"),
			// literal
			FMT_COLOR(FMT_RED), FMT_LIT("|.literal: "),
			FMT_COLOR(FMT_GREEN), FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"), FMT_LIT("\n"),
			FMT_COLOR(FMT_RED), FMT_LIT("|          "),
			FMT_LIT("", .width = (i16)parse.at + 1), FMT_LIT("^ here"), FMT_LIT("\n"),
			// end
			FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
			// debug
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
			FMT_COLOR(FMT_RESET),
		).ptr != NULL;
		case ARGS_MEM_PARSE_INVALID_SCALE: return fmt_write(state->out,
			// message
			FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("invalid scale in memory size, allow only B, KB, MB and GB"), FMT_LIT("\n"),
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_COLOR(FMT_GREEN), FMT_STR(state->peek), FMT_LIT("\n"),
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LIT(" ", .repeat = parse.at),
			FMT_LIT("^", .repeat = suffix_len), FMT_LIT(" here"), FMT_LIT("\n"),
			// debug
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
			FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
			FMT_COLOR(FMT_RESET),
		).ptr != NULL;
		default: PANIC("unhandled or invalid parse kind (%u)", parse.kind);
	};
};

#define args_mem_min_limit(state, mem) args_expect_mem_min_limit_ex(CALL, state, mem)
bool args_expect_mem_min_limit_ex(CallLoc call, ArgsState * state, ArgsMem const * mem) {
	return fmt_write(state->out,
		// message
		FMT_COLOR(FMT_BOLD ";" FMT_RED), FMT_LIT("[risk:error] "),
		FMT_COLOR(FMT_WHITE),  FMT_LIT("minimal "),
		FMT_COLOR(FMT_GREEN),  FMT_LIT("'--mem-limit'"),
		FMT_COLOR(FMT_WHITE),  FMT_LIT(" is "),
		FMT_COLOR(FMT_CYAN),   FMT_MEM(mem->min),
		FMT_COLOR(FMT_WHITE),  FMT_LIT(", but you set to "),
		FMT_COLOR(FMT_RED),    FMT_MEM(mem->max), FMT_LIT("\n"),
		// debug
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	).ptr != NULL;
};

ArgsResult args_mem_try(ArgsState * state, ArgsMem * mem) {
	if (!str_eq_lit(state->peek, "-m") && !str_eq_lit(state->peek, "--mem-limit")) return ARGS_UNKNOWN;
	args_state_skip(state);

	if (args_state_eof(state)) {
		args_mem_expected(state, mem);
		return ARGS_ERROR;
	};

	ArgsMemParse parse;
	if (!args_mem_parse(&mem->max, state->peek.ptr, &parse)) {
		args_mem_parse_fail(state, parse);
		return ARGS_ERROR;
	};

	if (mem->max < mem->min) {
		args_mem_min_limit(state, mem);
		return ARGS_ERROR;
	};

	args_state_skip(state);
	return ARGS_KNOWN;
};

