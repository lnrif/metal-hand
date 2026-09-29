#include "risk/src/ast/msg.h"

// |================================================================================================|
// |> PARSER MESSAGES                                                                               |

void parser_expected_ex(FlowLoc loc, Parser * parser, LexToken token, Str expect) {
	// if (parser->error) return;
	// parser->error = true;

	if (token.tag == LEX_T_EOF) {
		// if (parser->at != 0) token = parser_token(parser, parser->at - 1);
		SrcDot const dot = src_dot(parser->ast.src.str, SRC_DOT_NIL, token.pos);

		FMT(parser->fmt,
			FMT_BOLD, FMT_RED, FMT_LIT("[E] "),
			FMT_WHITE, FMT_LIT("expected "),
			FMT_GREEN, FMT_STR(expect),
			FMT_WHITE, FMT_LIT(", but found "),
			FMT_RED, FMT_LIT("end fo file"),
			FMT_LIT("\n"),
			FMT_RED, FMT_LIT("  | "), FMT_LOC(parser->ast.src.path, dot.row, dot.col),
			FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(loc),
			FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC),
			FMT_LIT("\n"), FMT_RESET,
		);

		return;
	};

	SrcDot const dot = src_dot(parser->ast.src.str, SRC_DOT_NIL, token.pos);
	Str const lexeme = STR(&parser->ast.src.raw[token.pos], token.len);

	FMT(parser->fmt,
		FMT_BOLD, FMT_RED, FMT_LIT("[E] "),
		FMT_WHITE, FMT_LIT("expected "),
		FMT_GREEN, FMT_STR(expect),
		FMT_WHITE, FMT_LIT(", but found "),
		FMT_RED, FMT_LIT("`"), FMT_STR(lexeme, .opt = FMT_S_ESCAPE), FMT_LIT("`\n"),
		FMT_RED, FMT_LIT("  | "),
		FMT_LOC(parser->ast.src.path, dot.row, dot.col),
		FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(loc),
		FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC),
		FMT_LIT("\n"), FMT_RESET,
	);
};

void parser_expected_after_ex(FlowLoc loc, Parser * parser, LexToken token, Str expected) {
	// if (parser->error) return;
	// parser->error = true;

	SrcDot const dot = src_dot(parser->ast.src.str, SRC_DOT_NIL, token.pos);
	Str const lexeme = STR(&parser->ast.src.raw[token.pos], token.len);

	FMT(parser->fmt,
		FMT_BOLD, FMT_RED, FMT_LIT("[E] "),
		FMT_WHITE, FMT_LIT("expected "), FMT_STR(expected), FMT_LIT(" after token "),
		FMT_RED, FMT_LIT("`"), FMT_STR(lexeme), FMT_LIT("`"), FMT_LIT("\n"),
		FMT_RED, FMT_LIT("  | "), FMT_LOC(parser->ast.src.path, dot.row, dot.col),
		FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(loc),
		FMT_RED, FMT_LIT("  | "), FMT_LOC_DEBUG(FLOW_LOC),
		FMT_LIT("\n"), FMT_RESET,
	);
};


