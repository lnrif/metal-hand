#ifndef STD_FLOW_CORE_H
#define STD_FLOW_CORE_H

#include "std/core.h"
#include "std/str/str.h"
#include "std/stream/stream.h"

// |================================================================================================|
// |> FLOW                                                                                          |
// |================================================================================================|

// |================================================================================================|
// |> location                                                                                      |

typedef struct { u8z const * get; } Flow_Loc;
typedef struct { u8z const * get; } Flow_Tag;

#define SECTION(name) __attribute__((section(name)))

#define FLOW_LOC_LIT __FILE__ ":" STRIFY(__LINE__)
#define FLOW_LOC      ((Flow_Loc){.get = (void*)FLOW_LOC_LIT})
#define FLOW_TAG(tag) ((Flow_Tag){.get = (void*)tag})

// |================================================================================================|
// |> statistics                                                                                    |

typedef struct {
	Flow_Tag tag; Flow_Loc loc; u64 cnt;
	struct { u64 sum; u64 min; u64 max; } total;
	struct { u64 sum; u64 min; u64 max; } nested;
} Flow_Stat;

#define FLOW_STAT_INIT(_tag, _loc) \
	((Flow_Stat){ \
		.tag = FLOW_TAG(_tag), .loc = (_loc), .cnt = 0, \
		.total  = {.sum = 0, .min = U64_MAX, .max = 0}, \
		.nested = {.sum = 0, .min = U64_MAX, .max = 0}, \
	})

// typedef struct {
// 	Flow_Stat * ptr; u64 len;
// 	Reg_Man man;
// } Flow_Stats;
//
// #define FLOW_STATS_RAW(_ptr, _len, _man) \
// 	((Flow_Stats){ \
// 		.ptr = (_ptr), .len = (_len), \
// 		.man = (_man), \
// 	})
//
// #define FLOW_STATS(man) FLOW_STATS_RAW(0, 0, (man))
// #define FLOW_STATS_NIL FLOW_STATS(REG_MAN_NIL)

#if LINUX
	#define FLOW_STAT(tag) \
		({ \
			static Flow_Stat SECTION("flow_stats") USED flow_stat = FLOW_STAT_INIT(tag, FLOW_LOC); \
			&flow_stat; \
		})
#else
	#error "todo: support `flow_items` section"
#endif

// |================================================================================================|
// |> call stack                                                                                    |

typedef struct { Flow_Stat * stat; Flow_Loc loc; u64 beg; u64 nested; } Flow_Call;

#define FLOW_CALL(_stat, _loc, _beg) \
	((Flow_Call){.stat = (_stat), .loc = (_loc), .beg = (_beg), .nested = 0})

// typedef struct {
// 	Flow_Call * ptr; u64 len; u64 cap;
// 	Reg_Man man; u64 lim;
// } Flow_Calls;
//
// #define FLOW_CALLS_RAW(_ptr, _len, _cap, _man, _lim) \
// 	((Flow_Calls){ \
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
// 		Flow_Calls calls;
// 		Flow_Stats stats;
// 		u8 peek[];
// 	} Flow_State;
// #else
// 	#error "todo: thread_local"
// #endif
//
// Flow_State * flow_state(void);
// #define FLOW (*flow_state())

// |================================================================================================|
// |> init                                                                                          |

// typedef struct { u64 get; } Flow_Calls_Limit;
// #define FLOW_CALLS_LIMIT(_get) ((Flow_Calls_Limit){.get = (_get)})

// b8 flow_init(Reg_Man calls, Reg_Man stats, Flow_Calls_Limit limit);
b8 flow_init();

// |================================================================================================|
// |> free                                                                                          |

void flow_free(void);

// |================================================================================================|
// |> enter                                                                                         |

typedef struct { Flow_Stat * stat; Flow_Loc call; Flow_Loc loc; } Flow_Enter;
#define FLOW_ENTER(_stat, _call) ((Flow_Enter){.stat = (_stat), .call = (_call), .loc = FLOW_LOC})

void flow_enter_ex(Flow_Enter enter);
#define flow_enter(stat, call) flow_enter_ex(FLOW_ENTER(stat, call))

// |================================================================================================|
// |> leave                                                                                         |

typedef struct { Flow_Stat * stat; Flow_Loc loc; } Flow_Leave;
#define FLOW_LEAVE(_stat) ((Flow_Leave){.stat = (_stat), .loc = FLOW_LOC})

void flow_leave_ex(Flow_Leave leave);
#define flow_leave(stat) flow_leave_ex(FLOW_LEAVE(stat))

// |================================================================================================|
// |> panic                                                                                         |

typedef struct { Flow_Loc loc; Str msg; } Flow_Panic;
#define FLOW_PANIC_RAW(_loc, _msg...) ((Flow_Panic){.loc = (_loc), .msg = (_msg)})
#define FLOW_PANIC(msg...) FLOW_PANIC_RAW(FLOW_LOC, S(msg))

// not use [flow_state]
void NO_RETURN flow_panic_raw(Flow_Panic panic);
// use [flow_state]
void NO_RETURN flow_panic(Flow_Panic panic);

#define PANIC_RAW(msg...) flow_panic_raw(FLOW_PANIC(msg))
#define PANIC(msg...)     flow_panic(FLOW_PANIC(msg))
#define PANIC_AT(loc, msg...) flow_panic(FLOW_PANIC_RAW(loc, S(msg)))

// |================================================================================================|
// |> assert                                                                                        |

// #define ASSERT(flow, ...)

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
	RET WRAP(Flow_Loc FLOW_CALLER, ARGS) { \
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
// static Flow_Tag INC_TAG = FLOW_TAG_NIL;
//
// u64 inc_wrap(Flow_Loc loc, u64 num) {
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
