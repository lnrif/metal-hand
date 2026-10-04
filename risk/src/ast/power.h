#ifndef RK_AST_POWER_H
#define RK_AST_POWER_H

#include "std/core.h"

// |================================================================================================|
// |> PARSER INFIX                                                                                  |

// #define PARSE_POWER_PREFIX(NAME) \
// 	PARSE_POWER_##NAME##_PREFIX

#define PARSE_POWER_LHS(NAME) \
	PARSE_POWER_INFIX_##NAME##_LHS, \
	PARSE_POWER_INFIX_##NAME##_RHS, \
	PARSE_POWER_INFIX_##NAME##_FINISH

#define PARSE_POWER_RHS(NAME) \
	PARSE_POWER_INFIX_##NAME##_RHS, \
	PARSE_POWER_INFIX_##NAME##_LHS, \
	PARSE_POWER_INFIX_##NAME##_FINISH

// #define PARSE_POWER_PREFIX_AS(AS, NAME) \
// 	PARSE_POWER_INFIX_##NAME##_PREFIX = PARSE_POWER_INFIX_##AS##_PREFIX

#define PARSE_POWER_INFIX_AS(AS, NAME) \
	PARSE_POWER_INFIX_##NAME##_LHS    = PARSE_POWER_INFIX_##AS##_LHS, \
	PARSE_POWER_INFIX_##NAME##_RHS    = PARSE_POWER_INFIX_##AS##_RHS, \
	PARSE_POWER_INFIX_##NAME##_FINISH = PARSE_POWER_INFIX_##AS##_FINISH \

typedef enum: u8 {
	PARSE_POWER_NIL,

	// // if a
	// PARSE_POWER_PREFIX_IF,
	// // a then b
	// PARSE_POWER_LHS(THEN),
	// 	// a elif b
	// 	PARSE_POWER_INFIX_AS(THEN, ELIF),
	// 	// a else b
	// 	PARSE_POWER_INFIX_AS(THEN, ELSE),

	// a = b = c; a = (b = c)
	PARSE_POWER_RHS(SET),
		// a := b := c; a := (b := c)
		PARSE_POWER_INFIX_AS(SET, DEF),
		// a => b => c; a => (b => c)
		PARSE_POWER_INFIX_AS(SET, IMPLIES),

	// a <| b <| c; a <| (b <| c)
	PARSE_POWER_RHS(APPLY),
	// a :: b :: c; a :: (b :: c)
	PARSE_POWER_RHS(EXTEND),
	// a : b : c; a : (b : c)
	PARSE_POWER_RHS(COLON),

	// a -> b { c } == (a -> b) { c }
	PARSE_POWER_CALL_BRACES,

	// a -> b -> c; a -> (a -> b)
	PARSE_POWER_RHS(ARROW),

	// a || b || c; (a || b) || c
	PARSE_POWER_LHS(LOG_OR),
	// a && b && c; (a && b) && c
	PARSE_POWER_LHS(LOG_AND),

	// a == b == c; (a == b) == c
	PARSE_POWER_LHS(EQ),
		// a != b != c; (a != b) != c
		PARSE_POWER_INFIX_AS(EQ, NEQ),

	// a < b < c; (a < b) < c
	PARSE_POWER_LHS(LT),
		// a <= b <= c; (a <= b) <= c
		PARSE_POWER_INFIX_AS(LT, LE),
		// a > b > c; (a > b) > c
		PARSE_POWER_INFIX_AS(LT, GT),
		// a >= b >= c; (a >= b) >= c
		PARSE_POWER_INFIX_AS(LT, GE),

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
	PARSE_POWER_LHS(ADD),
		// a - b - c; (a - b) - c
		PARSE_POWER_INFIX_AS(ADD, SUB),

	// a * b * c == (a * b) * c
	PARSE_POWER_LHS(MUL),
		// a / b / c == (a / b) / c
		PARSE_POWER_INFIX_AS(MUL, DIV),

	// +a
	PARSE_POWER_PREFIX_POS,
	// -a
	PARSE_POWER_PREFIX_NEG = PARSE_POWER_PREFIX_POS,
	// !a
	PARSE_POWER_PREFIX_NOT = PARSE_POWER_PREFIX_POS,
	// .a
	PARSE_POWER_PREFIX_DOT = PARSE_POWER_PREFIX_POS,

	// x * f()  => x * (f())
	PARSE_POWER_CALL_PARENS,
	// x * f[]  => x * (f[])
	PARSE_POWER_CALL_BRACKTES   = PARSE_POWER_CALL_PARENS,
	// x * f.{} => x * (f.{})
	// x * f {} => (x * f) {}
	//      ^ no dot here
	PARSE_POWER_CALL_DOT_BRACES = PARSE_POWER_CALL_PARENS,

		// a.b.c; (a.b).c
	PARSE_POWER_LHS(DOT),
		// a'b'c; (a'b)'c
		PARSE_POWER_INFIX_AS(DOT, TICK),
} ParsePower;

#endif // !RK_AST_POWER_H
