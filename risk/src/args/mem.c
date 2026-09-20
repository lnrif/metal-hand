#include "risk/src/args/args.h"
#include "std/flow/core.h"
#include "std/fmt/core.h"

////////////////////////////////
// parse

typedef enum: u8 {
	ARGS_MEM_PARSE_OK,
	ARGS_MEM_PARSE_EMPTY,
	ARGS_MEM_PARSE_OVERFLOW,
	ARGS_MEM_PARSE_INVALID_DIGIT,
	ARGS_MEM_PARSE_INVALID_SCALE,
} ArgsMemParseKind;

typedef struct {
	u32 at;
	ArgsMemParseKind kind;
} ArgsMemParse;

b8 args_mem_parse(u64 * x, u8 const * z, ArgsMemParse * parse) {
	ASSERT_PTR(x);
	ASSERT_PTR(z);
	ASSERT_PTR(parse);

	if (*z == '\0') {
		parse->kind = ARGS_MEM_PARSE_EMPTY;
		return false;
	};

	u64 a = 0; u32 i = 0;
	if (z[i] < '0' || '9' < z[i]) {
		parse->kind = ARGS_MEM_PARSE_INVALID_DIGIT; parse->at = i;
		return false;
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
	//    |
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
	return true;

invalid_scale:
	parse->kind = ARGS_MEM_PARSE_INVALID_SCALE; parse->at = i;
	return false;

overflow:
	parse->kind = ARGS_MEM_PARSE_OVERFLOW; parse->at = i;
	return false;
};

////////////////////////////////
// diag

#define args_mem_expected(state, mem) args_mem_expected_ex(FLOW_LOC, state, mem)
b8 args_mem_expected_ex(FlowLoc flow, ArgsState * state, ArgsMem const * mem) {
	return FMT(state->out,
		// message
		FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
		FMT_MAGENTA, FMT_LIT("'--mem-limit'"),
		FMT_WHITE, FMT_LIT(" expect memory size, for example default is "),
		FMT_MAGENTA, FMT_LIT("'--mem-limit "), FMT_MEM(mem->min), FMT_LIT("'"), FMT_LIT("\n"),
		// debug
		FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
		FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
		FMT_RESET,
	);
};

#define args_mem_parse_fail(state, parse) args_mem_parse_fail_ex(FLOW_LOC, state, parse)
b8 args_mem_parse_fail_ex(FlowLoc flow, ArgsState * state, ArgsMemParse parse) {
	u32 const suffix_len = (u32)state->peek.len - parse.at;
	switch (parse.kind) {
		case ARGS_MEM_PARSE_OK: return true;
		case ARGS_MEM_PARSE_EMPTY: return FMT(state->out,
			// message
			FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
			FMT_WHITE, FMT_LIT("memory size is empty, please, provide"), FMT_LIT("\n"),
			// debug
			FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
			FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
			FMT_RESET,
		);
		case ARGS_MEM_PARSE_OVERFLOW: return FMT(state->out,
			// message
			FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
			FMT_WHITE, FMT_LIT("memory size overflow"), FMT_LIT("\n"),
			// literal
			FMT_RED, FMT_LIT("  |.literal: "),
			FMT_GREEN, FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"), FMT_LIT("\n"),
			FMT_RED, FMT_LIT("  |          "),
			FMT_LIT("", .width = (u32)parse.at + 1), FMT_LIT("^ overflow here"), FMT_LIT("\n"),
			// end
			FMT_RED, FMT_LIT("'\n"),
			// debug
			FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
			FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
			FMT_RESET,
		);
		case ARGS_MEM_PARSE_INVALID_DIGIT: return FMT(state->out,
			// message
			FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
			FMT_WHITE, FMT_LIT("invalid digit in memory size"), FMT_LIT("\n"),
			// literal
			FMT_RED, FMT_LIT("  |.literal: "),
			FMT_GREEN, FMT_LIT("'"), FMT_STR(state->peek), FMT_LIT("'"), FMT_LIT("\n"),
			FMT_RED, FMT_LIT("  |          "),
			FMT_LIT("", .width = (u32)parse.at + 1), FMT_LIT("^ here"), FMT_LIT("\n"),
			// end
			FMT_RED, FMT_LIT("'\n"),
			// debug
			FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
			FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
			FMT_RESET,
		);
		case ARGS_MEM_PARSE_INVALID_SCALE: return FMT(state->out,
			// message
			FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
			FMT_WHITE, FMT_LIT("invalid scale in memory size, allow only B, KB, MB and GB"), FMT_LIT("\n"),
			FMT_RED, FMT_LIT("  | "), FMT_GREEN, FMT_STR(state->peek), FMT_LIT("\n"),
			FMT_RED, FMT_LIT("  | "), FMT_REPEAT(' ', parse.at),
			FMT_REPEAT('^', suffix_len), FMT_LIT(" here"), FMT_LIT("\n"),
			// debug
			FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
			FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
			FMT_RESET,
		);
		default: PANIC("unhandled or invalid parse kind"); // parse.kind
	};
};

#define args_mem_min_limit(state, mem) args_expect_mem_min_limit_ex(FLOW_LOC, state, mem)
b8 args_expect_mem_min_limit_ex(FlowLoc flow, ArgsState * state, ArgsMem const * mem) {
	return FMT(state->out,
		// message
		FMT_BOLD, FMT_RED, FMT_LIT("[e] "),
		FMT_WHITE, FMT_LIT("minimal "),
		FMT_GREEN, FMT_LIT("'--mem-limit'"),
		FMT_WHITE, FMT_LIT(" is "),
		FMT_CYAN, FMT_MEM(mem->min),
		FMT_WHITE, FMT_LIT(", but you set to "),
		FMT_RED, FMT_MEM(mem->max), FMT_LIT("\n"),
		// debug
		FMT_RED, FMT_LIT("  | "), FMT_LOC(flow),
		FMT_RED, FMT_LIT("  | "), FMT_LOC(FLOW_LOC),
		FMT_RESET,
	);
};

ArgsResult args_mem_try(ArgsState * state, ArgsMem * mem) {
	if (!str_eq_lit(state->peek, "-m") && !str_eq_lit(state->peek, "--mem-limit")) return ARGS_UNKNOWN;
	arg_skip(&state->args);

	if (str_nil(state->peek)) {
		args_mem_expected(state, mem);
		return ARGS_ERROR;
	};

	ArgsMemParse parse;
	if (!args_mem_parse(&mem->max, state->peek.raw, &parse)) {
		args_mem_parse_fail(state, parse);
		return ARGS_ERROR;
	};

	if (mem->max < mem->min) {
		args_mem_min_limit(state, mem);
		return ARGS_ERROR;
	};

	arg_skip(&state->args);
	return ARGS_KNOWN;
};

