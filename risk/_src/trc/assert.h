#ifndef RK_ABORT_H
#define RK_ABORT_H

#include "core/core.h" // IWYU pragma: keep

////////////////////////////////////////////////////////////////////////////////
// abort

#include <stdlib.h>
#include <stdio.h> // IWYU pragma: keep

#define WHAT printf("%s:%u (%s)\n", __FILE__, __LINE__, __func__)
#define abort() do { exit(EXIT_FAILURE); } while (0)

typedef struct {
	char const * path;
	char const * func;
	u32 line;
} CallLoc;

#define ASSERT_RED    "\x1b[0;1;31m"
// #define ASSERT_ORANGE "\x1b[0;1;38;5;214m"
// #define ASSERT_BLUE   "\x1b[0;1;34m"
// // #define TERM_GREEN  "\x1b[0;1;32m"
#define ASSERT_WHITE  "\x1b[0;1;37m"
#define ASSERT_GREY   "\x1b[0;38;5;8m"
#define ASSERT_CYAN   "\x1b[0;36m"
#define ASSERT_PURPLE "\x1b[0;1;35m"
#define ASSERT_RESET  "\x1b[0m"

#define CALL \
	((CallLoc){.path = __FILE__, .func = __func__, .line = __LINE__})

#define ABORT(kind, default_text, fmt, ...) do { \
	if (sizeof(fmt) > 1) { \
		fprintf(stderr, ASSERT_RED "%s: " ASSERT_WHITE fmt "\n", kind, ##__VA_ARGS__); \
	} else { \
		fprintf(stderr, ASSERT_RED "%s: " ASSERT_WHITE "%s\n", kind, default_text); \
	}; \
	fprintf(stderr, ASSERT_GREY " ->> %s:%d " ASSERT_PURPLE "(%s)\n" ASSERT_RESET, __FILE__, __LINE__, __func__); \
	abort(); \
} while (0)

#define ABORT_AT(call, kind, default_text, fmt, ...) do { \
	if (sizeof(fmt) > 1) { \
		fprintf(stderr, ASSERT_RED "%s: " ASSERT_WHITE fmt "\n", kind, ##__VA_ARGS__); \
	} else { \
		fprintf(stderr, ASSERT_RED "%s: " ASSERT_WHITE "%s\n", kind, default_text); \
	} \
	fprintf(stderr, ASSERT_GREY " ->> %s:%d " ASSERT_PURPLE "(%s)\n", call.path, call.line, call.func); \
	fprintf(stderr, ASSERT_GREY " ->> %s:%d " ASSERT_PURPLE "(%s)\n", __FILE__, __LINE__, __func__); \
	abort(); \
} while (0)

////////////////////////////////////////////////////////////////////////////////
// wrapper abort

#define ASSERT(cond, fmt, ...) do { \
	if (!(cond)) ABORT("assert", #cond, fmt, ##__VA_ARGS__); \
} while (0)

#define PANIC(fmt, ...) \
	ABORT("panic", "something went wrong!", fmt, ##__VA_ARGS__)

#define TODO(fmt, ...) \
	ABORT("todo", "not implemented!", fmt, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
// wrapper abort at

#define ASSERT_AT(call, cond, fmt, ...) do { \
	if (!(cond)) ABORT_AT(call, "assert", #cond, fmt, ##__VA_ARGS__); \
} while (0)

#define PANIC_AT(call, fmt, ...) \
	ABORT_AT(call, "panic", "something went wrong!", fmt, ##__VA_ARGS__)

#define TODO_AT(call, fmt, ...) \
	ABORT_AT(call, "todo", "not implemented!", fmt, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
// debug

#define ASSERT_DEBUG_ENABLE

#ifdef ASSERT_DEBUG_ENABLE
	#define ASSERT_DEBUG(cond, fmt, ...) \
		ASSERT(cond, fmt, ##__VA_ARGS__)
	#define ASSERT_DEBUG_AT(call, cond, fmt, ...) \
		ASSERT_AT(call, cond, fmt, ##__VA_ARGS__)
#else
	#define ASSERT_DEBUG(cond, fmt, ...) \
		do { (void)(cond); } while (0)
	#define ASSERT_DEBUG_AT(call, cond, fmt, ...) \
		do { (void)(cond); } while (0)
#endif

////////////////////////////////////////////////////////////////////////////////
// debug helpers

#define ALIGN_IS_SANE(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

#define ASSERT_DEBUG_MUL(call, a, b, r) \
	ASSERT_DEBUG_AT( \
		call, !MUL_OVER_DBG(a, b, r), \
		"overflow: (" #a ": %llu) * (" #b ": %llu)", \
		a, b \
	)

#define ASSERT_DEBUG_ADD(call, a, b, r) \
	ASSERT_DEBUG_AT( \
		call, !ADD_OVER_DBG(a, b, r), \
		"overflow: (" #a ": %llu) + (" #b ": %llu)", \
		a, b \
	)

#define ADD(call, a, b) \
	({ \
		typeof(a) _result;\
		ASSERT_DEBUG_AT( \
			call, !ADD_OVER_DBG(a, b, &_result), \
			"overflow: (" #a ": %llu) + (" #b ": %llu)", \
			a, b \
		); \
		_result; \
	})

#define MUL(call, a, b) \
	({ \
		typeof(a) _result;\
		ASSERT_DEBUG_AT( \
			call, !MUL_OVER_DBG(a, b, &_result), \
			"overflow: (" #a ": %llu) * (" #b ": %llu)", \
			a, b \
		); \
		_result; \
	})

#define ASSERT_DEBUG_PTR(ptr) \
	ASSERT_DEBUG((ptr) != NULL, "expected non-null [" #ptr "]")

#define ASSERT_DEBUG_STR(str) \
	ASSERT_DEBUG((str).ptr != NULL, "expected non-null [" #str "]")

#endif // !RK_ABORT_H
