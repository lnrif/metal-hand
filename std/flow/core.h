#ifndef STD_FLOW_CORE_H
#define STD_FLOW_CORE_H

#include "std/core.h"
#include "std/str/core.h"

// |================================================================================================|
// |> FLOW                                                                                          |
// |================================================================================================|

// |================================================================================================|
// |> location                                                                                      |

typedef struct { u8z const * get; } FlowLoc;

#define FLOW_LOC_LIT __FILE__ ":" STRIFY(__LINE__)
#define FLOW_LOC      ((FlowLoc){.get = (void*)FLOW_LOC_LIT})

// |================================================================================================|
// |> panic                                                                                         |

typedef struct { FlowLoc loc; Str msg; } FlowPanic;
#define FLOW_PANIC_RAW(_loc, _msg...) ((FlowPanic){.loc = (_loc), .msg = (_msg)})
#define FLOW_PANIC(msg...) FLOW_PANIC_RAW(FLOW_LOC, S(msg))

// use [flow_state]
void NO_RETURN flow_panic(FlowPanic panic);

#define PANIC(msg...)         flow_panic(FLOW_PANIC(msg))
#define PANIC_AT(loc, msg...) flow_panic(FLOW_PANIC_RAW(loc, S(msg)))

// |================================================================================================|
// |> assert                                                                                        |

#define ASSERT(cond, msg...) \
	do { \
		if (cond) break; \
		PANIC(msg); \
	} while (0)

#define ASSERT_RAW(cond, msg...) \
	do { \
		if (cond) break; \
		PANIC_RAW(msg); \
	} while (0)

#define ASSERT_ALIGN(align) \
	ASSERT(ALIGN_IS_SANE(align), "invalid align [" #align "]")

#define ASSERT_ALLOC(align, size) \
	ASSERT( \
		ALIGN_IS_SANE(align) && ((size) <= (align) || (size) % (align) == 0), \
		"invalid allocation { align: " STRIFY(align) ", size: " STRIFY(size) "}" \
	)

#define ASSERT_PTR(ptr) \
	ASSERT((ptr) != 0, "expected non-null [" #ptr "]")
#define ASSERT_STR(call,str) \
	ASSERT((str).ptr != 0, "expected non-null [" #str "]")
#define ASSERT_SLICE(ptr, len) \
	ASSERT((ptr) != 0 || ((ptr) == 0 && (len) == 0), "invalid slice { ptr: " STRIFY(ptr) ", len: " STRIFY(len) " }")

#define ADD(T, a, b) \
	({ \
		T _result;\
		ASSERT(!ADD_OVER_DBG(a, b, &_result), "overflow: " STRIFY(a) " + " STRIFY(a)); \
		_result; \
	})

#define MUL(T, a, b) \
	({ \
		T _result;\
		ASSERT(!MUL_OVER_DBG(a, b, &_result), "overflow: " STRIFY(a) " * " STRIFY(b)); \
		_result; \
	})

#endif // !STD_FLOW_CORE_H
