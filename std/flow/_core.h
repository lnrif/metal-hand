#ifndef STD_FLOW_CORE_H
#define STD_FLOW_CORE_H

#include "std/core.h"
#include "std/str/core.h"
#include "std/stream/core.h"

// |================================================================================================|
// |> FLOW                                                                                          |
// |================================================================================================|

// |================================================================================================|
// |> location                                                                                      |

typedef struct { u8z const * get; } FlowLoc;
typedef struct { u8z const * get; } FlowTag;

#define SECTION(name) __attribute__((section(name)))

#define FLOW_LOC_LIT __FILE__ ":" STRIFY(__LINE__)
#define FLOW_LOC      ((FlowLoc){.get = (void*)FLOW_LOC_LIT})
#define FLOW_TAG(tag) ((FlowTag){.get = (void*)tag})

// |================================================================================================|
// |> statistics                                                                                    |

typedef struct {
	FlowTag tag; FlowLoc loc; u64 cnt;
	struct { u64 sum; u64 min; u64 max; } total;
	struct { u64 sum; u64 min; u64 max; } nested;
} FlowStat;

#define FLOW_STAT_INIT(_tag, _loc) \
	((FlowStat){ \
		.tag = FLOW_TAG(_tag), .loc = (_loc), .cnt = 0, \
		.total  = {.sum = 0, .min = U64_MAX, .max = 0}, \
		.nested = {.sum = 0, .min = U64_MAX, .max = 0}, \
	})

// typedef struct {
// 	FlowStat * ptr; u64 len;
// 	Reg_Man man;
// } FlowStats;
//
// #define FLOW_STATS_RAW(_ptr, _len, _man) \
// 	((FlowStats){ \
// 		.ptr = (_ptr), .len = (_len), \
// 		.man = (_man), \
// 	})
//
// #define FLOW_STATS(man) FLOW_STATS_RAW(0, 0, (man))
// #define FLOW_STATS_NIL FLOW_STATS(REG_MAN_NIL)

#if LINUX
	#define FLOW_STAT(tag) \
		({ \
			static FlowStat SECTION("flow_stats") USED flow_stat = FLOW_STAT_INIT(tag, FLOW_LOC); \
			&flow_stat; \
		})
#else
	#error "todo: support `flow_items` section"
#endif

// |================================================================================================|
// |> call stack                                                                                    |

typedef struct { FlowStat * stat; FlowLoc loc; u64 beg; u64 nested; } FlowCall;

#define FLOW_CALL(_stat, _loc, _beg) \
	((FlowCall){.stat = (_stat), .loc = (_loc), .beg = (_beg), .nested = 0})

// typedef struct {
// 	FlowCall * ptr; u64 len; u64 cap;
// 	Reg_Man man; u64 lim;
// } FlowCalls;
//
// #define FLOW_CALLS_RAW(_ptr, _len, _cap, _man, _lim) \
// 	((FlowCalls){ \
// 		.ptr = (_ptr), .len = (_len), .cap = (_cap), \
// 		.man = (_man), .lim = (_lim), \
// 	})
//
// #define FLOW_CALLS(man, lim) FLOW_CALLS_RAW(0, 0, 0, (man), (lim))
// #define FLOW_CALLS_NIL FLOW_CALLS(REG_MAN_NIL, 0)


// |================================================================================================|
// |> state                                                                                         |

// #if LINUX && X86_64
// 	typedef struct {
// 		void * thread_local; // FS base
// 		FlowCalls calls;
// 		FlowStats stats;
// 		u8 peek[];
// 	} FlowState;
// #else
// 	#error "todo: thread_local"
// #endif
//
// FlowState * flow_state(void);
// #define FLOW (*flow_state())

// |================================================================================================|
// |> init                                                                                          |

// typedef struct { u64 get; } FlowCalls_Limit;
// #define FLOW_CALLS_LIMIT(_get) ((FlowCalls_Limit){.get = (_get)})

// b8 flow_init(Reg_Man calls, Reg_Man stats, FlowCalls_Limit limit);
b8 flow_init();

// |================================================================================================|
// |> free                                                                                          |

void flow_free(void);

// |================================================================================================|
// |> enter                                                                                         |

typedef struct { FlowStat * stat; FlowLoc call; FlowLoc loc; } FlowEnter;
#define FLOW_ENTER(_stat, _call) ((FlowEnter){.stat = (_stat), .call = (_call), .loc = FLOW_LOC})

void flow_enter_ex(FlowEnter enter);
#define flow_enter(stat, call) flow_enter_ex(FLOW_ENTER(stat, call))

// |================================================================================================|
// |> leave                                                                                         |

typedef struct { FlowStat * stat; FlowLoc loc; } FlowLeave;
#define FLOW_LEAVE(_stat) ((FlowLeave){.stat = (_stat), .loc = FLOW_LOC})

void flow_leave_ex(FlowLeave leave);
#define flow_leave(stat) flow_leave_ex(FLOW_LEAVE(stat))

// |================================================================================================|
// |> panic                                                                                         |

typedef struct { FlowLoc loc; Str msg; } FlowPanic;
#define FLOW_PANIC_RAW(_loc, _msg...) ((FlowPanic){.loc = (_loc), .msg = (_msg)})
#define FLOW_PANIC(msg...) FLOW_PANIC_RAW(FLOW_LOC, S(msg))

// not use [flow_state]
void NO_RETURN flow_panic_raw(FlowPanic panic);
// use [flow_state]
void NO_RETURN flow_panic(FlowPanic panic);

#define PANIC_RAW(msg...) flow_panic_raw(FLOW_PANIC(msg))
#define PANIC(msg...)     flow_panic(FLOW_PANIC(msg))
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

#define ALIGN_IS_SANE(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

#define ASSERT_ALIGN(align) \
	ASSERT(ALIGN_IS_SANE(align), "invalid align [" #align "]")
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


// |================================================================================================|
// |> out                                                                                           |

b8 flow_show_calls(Stream * out);
b8 flow_show_stats(Stream * out);

// |================================================================================================|
// |> wrapper                                                                                       |

#define FLOW_WRAPPER( \
	NAME, \
	WRAP, IMPL, TAG, \
	RET, CALL, ARGS... \
) \
	RET WRAP(FlowLoc FLOW_CALLER, ARGS) { \
		ENTER(&TAG, FLOW_CALLER, NAME); \
		RET const FLOW_RET = IMPL CALL; \
		LEAVE(TAG); \
		return FLOW_RET; \
	};

// u64 inc_impl(u64 num) {
// 	if (num == U64_MAX) PANIC("overflow!");
// 	return num + 1;
// };
//
// static FlowTag INC_TAG = FLOW_TAG_NIL;
//
// u64 inc_wrap(FlowLoc loc, u64 num) {
//
// };
//
// #define inc(num) inc_wrap(FLOW_LOC, num)
//
// FLOW_WRAPPER(
// 	"inc {x: u64} -> {y: u64}",
// 	inc, inc_impl, INC_TAG,
// 	u64, (num), u64 num
// );

// #define ADD(call, T, a, b) \
// 	({ \
// 		T _result;\
// 		ASSERT_RAW(call, !ADD_OVER_DBG(a, b, &_result), "overflow: " STRIFY(a) " + " STRIFY(a)); \
// 		_result; \
// 	})
//
// #define MUL(call, T, a, b) \
// 	({ \
// 		T _result;\
// 		ASSERT_RAW(call, !MUL_OVER_DBG(a, b, &_result), "overflow: " STRIFY(a) " * " STRIFY(b)); \
// 		_result; \
// 	})

#endif // !STD_FLOW_CORE_H
