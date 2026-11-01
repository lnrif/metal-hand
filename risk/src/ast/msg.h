#ifndef RK_AST_MSG_H
#define RK_AST_MSG_H

#include "std/flow/core.h"
#include "risk/src/ast/parser.h"

// |================================================================================================|
// |> PARSER MESSAGES                                                                               |

b8 parser_error_can_accept(Parser const * parser);
b8 parser_error_try_accept(Parser * parser);

#define parser_expected(parser, token, expect) parser_expected_ex(FLOW_LOC, parser, token, expect)
void parser_expected_ex(FlowLoc loc, Parser * parser, LexToken token, Str expect);

#define parser_expected_after(parser, token, expected) parser_expected_after_ex(FLOW_LOC, parser, token, expected)
void parser_expected_after_ex(FlowLoc loc, Parser * parser, LexToken token, Str expected);

#define parser_unterminated_string(parser, token) parser_unterminated_string_ex(FLOW_LOC, parser, token)
void parser_unterminated_string_ex(FlowLoc loc, Parser * parser, LexToken token);

#endif // !RK_AST_MSG_H

