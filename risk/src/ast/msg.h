#ifndef RK_AST_MSG_H
#define RK_AST_MSG_H

#include "std/flow/core.h"
#include "risk/src/ast/parser.h"

// |================================================================================================|
// |> PARSER MESSAGES                                                                               |

#define parser_expected(parser, token, expect) parser_expected_ex(FLOW_LOC, parser, token, expect)
void parser_expected_ex(FlowLoc loc, Parser * parser, LexToken token, Str expect);

#define parser_expected_after(parser, token, expected) parser_expected_after_ex(FLOW_LOC, parser, token, expected)
void parser_expected_after_ex(FlowLoc loc, Parser * parser, LexToken token, Str expected);

#endif // !RK_AST_MSG_H

