#include "std/flow/core.h"
// #include "std/mem/reg.h"
#include "std/proc/exit.h"
#include "std/stream/stream.h"

// |================================================================================================|
// |> global state                                                                                  |

// static Flow_Calls CALLS = FLOW_CALLS_NIL;
// static Flow_Stats STATS = FLOW_STATS_NIL;
static b8 IS_PANIC = false;

// #if LINUX && X86_64
//
// 	// [arch_prctl_set_fs]
// 	static void linux_arch_prctl_set_fs(void * ptr) { syscall2(158, 0x1002, (uptr)ptr); };
//
// 	static void * linux_read_ptr_from_fs(void) {
// 		void * ptr; asm volatile ("mov %%fs:0, %0" : "=r"(ptr));
// 		return ptr;
// 	};
//
// 	static void flow_base(void * ptr) { linux_arch_prctl_set_fs(ptr); };
// 	Flow_State * flow_state(void) { return linux_read_ptr_from_fs(); };
//
// #else
// 	#error "todo"
// #endif

#if LINUX
	extern Flow_Stat __start_flow_stats[];
	extern Flow_Stat  __stop_flow_stats[];
	static Flow_Call CALLS[1024] = {0}; static u64 CALLS_LEN = 0;
#else
	#error "todo: support `flow_items` section"
#endif

// |================================================================================================|
// |> init                                                                                          |

b8 flow_init() {
	// CALLS = FLOW_CALLS(calls, limit.get);
	// STATS = FLOW_STATS(stats);
	// IS_PANIC = false;

	// #if LINUX && X86_64
	// 	u64 const stats_count = (u64)(__stop_flow_stats - __start_flow_stats);
	// #else
	// 	#error "todo"
	// #endif
	//
	// Reg const stats_reg = reg_alloc(STATS.man, typeof(*STATS.ptr), stats_count);
	// if (stats_reg.ptr == 0) return false;
	//
	// STATS.ptr = stats_reg.any;
	// STATS.len = stats_reg.len / sizeof(*STATS.ptr);

	return true;
};

// |================================================================================================|
// |> free                                                                                          |

void flow_free(void) {
	// reg_free(CALLS.man, CALLS.ptr, CALLS.cap);
	// CALLS.len = 0; CALLS.cap = 0; CALLS.ptr = 0;
	//
	// reg_free(STATS.man, STATS.ptr, STATS.len);
	// STATS.len = 0;                STATS.ptr = 0;
};

// |================================================================================================|
// |> enter                                                                                         |

b8 flow_reserve(u64 count) {
	if (CALLS_LEN + count <= LEN(CALLS)) return true;
	// reg_update(BUF.man, REG_VEC(BUF.ptr, BUF.len), REG_ADD(typeof(*BUF.ptr), BUF.len, REG_DIR_UP));
	// TODO: grow
	return false;
};

#if X86_64
	static u64 rdtsc(void) {
		u32 lo, hi;
		__asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
		return ((u64)hi << 32) | lo;
	};
#else
	#error "todo"
#endif

void flow_enter_ex(Flow_Enter enter) {
	if (!flow_reserve(1)) PANIC("OOM");
	CALLS[CALLS_LEN] = FLOW_CALL(enter.stat, enter.loc, rdtsc());
	CALLS_LEN += 1;
};

// |================================================================================================|
// |> leave                                                                                         |

void flow_leave_ex(Flow_Leave leave) {
	if (CALLS_LEN == 0) {
		PANIC_AT(leave.loc, "empty call stack");
	};

	if (CALLS[CALLS_LEN - 1].stat != leave.stat) {
		PANIC_AT(leave.loc, "dismatch call at stack and leave call");
	};

	Flow_Stat * stat = leave.stat;
	u64 const end = rdtsc();
	u64 const delta = end - CALLS[CALLS_LEN - 1].beg;

	stat->cnt += 1;

	stat->total.sum += delta;
	stat->total.min = MIN(stat->total.min, delta);
	stat->total.max = MAX(stat->total.max, delta);

	if (CALLS_LEN > 1) CALLS[CALLS_LEN - 2].nested += delta;

	stat->nested.sum += CALLS[CALLS_LEN - 1].nested;
	stat->nested.min = MIN(stat->nested.min, CALLS[CALLS_LEN - 1].nested);
	stat->nested.max = MAX(stat->nested.max, CALLS[CALLS_LEN - 1].nested);

	CALLS_LEN -= 1;
};

// |================================================================================================|
// |> panic                                                                                         |

void NO_RETURN flow_panic(Flow_Panic panic) {
	if (IS_PANIC) flow_panic_raw(panic);
	IS_PANIC = true;

	Stream err = stream_output();
	stream_write_arr(&err,
		SL("[panic] "), SS(panic.msg), SL("\n"),
		SL(" --> "), SZ(panic.loc.get), SL("\n"),
		SL(" --> "), SS(S(FLOW_LOC_LIT)), SL("\n\n"),
	);

	flow_show_calls(&err);

	proc_exit(1);
};

void NO_RETURN flow_panic_raw(Flow_Panic panic) {
	Stream err = stream_output();
	stream_write_arr(&err,
		SS(panic.msg), SL("\n"),
		SL(" --> "), SZ(panic.loc.get), SL("\n"),
		SL(" --> "), SS(S(FLOW_LOC_LIT)), SL("\n\n"),
	);
	proc_exit(2);
};

// |================================================================================================|
// |> out                                                                                           |

b8 flow_show_calls(Stream * out) {
	for (u64 i = CALLS_LEN; i > 0; i -= 1) {
		stream_write_arr(out,
			SL("[_] "),
			SZ(CALLS[i - 1].loc.get),
			SL(" | "),
			SZ(CALLS[i - 1].stat->tag.get),
			SL("\n"),
		);
		if (i - 1 == 0) stream_write_lit(out, "\n");
	};
	return true;
};

b8 flow_show_stats(Stream * out) {
	UNUSED(out);
	return false;
};

