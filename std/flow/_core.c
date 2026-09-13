#include "std/flow/core.h"
// #include "std/mem/reg.h"
#include "std/proc/exit.h"
#include "std/str/str.h"
#include "std/stream/stream.h"
#include "std/fmt/core.h"

// |================================================================================================|
// |> global state                                                                                  |

// static FLowCalls CALLS = FLOW_CALLS_NIL;
// static FLowStats STATS = FLOW_STATS_NIL;
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
// 	FLowState * flow_state(void) { return linux_read_ptr_from_fs(); };
//
// #else
// 	#error "todo"
// #endif

#if LINUX
	extern FLowStat __start_flow_stats[];
	extern FLowStat  __stop_flow_stats[];
	#define STATS_LEN (u64)(__stop_flow_stats - __start_flow_stats)

	static FLowCall CALLS[1024] = {0}; static u64 CALLS_LEN = 0;
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
		__asm__ volatile (
			"lfence\n"
			"rdtsc\n"
			: "=a"(lo), "=d"(hi)
		);
		return ((u64)hi << 32) | lo;
	};
#else
	#error "todo"
#endif

// static FLowStat SECTION("flow_stats") USED STAT = FLOW_STAT_INIT("flow {} -> {}", FLOW_LOC);

void flow_enter_ex(FLowEnter enter) {
	if (!flow_reserve(2)) PANIC("OOM");

	// FmtOut fmt = FMT_ON_STACK(64);
	//
	// fmt_u64(&fmt, (u64)enter.stat);
	// fmt_raw(&fmt, " ");
	// fmt_u64(&fmt, (u64)__start_flow_stats);
	// fmt_raw(&fmt, " ");
	// fmt_u64(&fmt, (u64)__stop_flow_stats);
	// fmt_raw(&fmt, "\x1B[36m" " --> " FLOW_LOC_LIT "\n");
	//
	// Stream out = stream_output();
	// stream_write_str(&out, fmt_as_str(&fmt));
	// fmt_reset(&fmt);
	//

	// CALLS[CALLS_LEN] = FLOW_CALL(&STAT, FLOW_LOC, rdtsc());
	// CALLS_LEN += 1;

	CALLS[CALLS_LEN] = FLOW_CALL(enter.stat, enter.loc, rdtsc());
	CALLS_LEN += 1;
};

// |================================================================================================|
// |> leave                                                                                         |

void flow_leave_impl(FLowLeave leave) {
	if (CALLS_LEN == 0) {
		PANIC_AT(leave.loc, "empty call stack");
	};

	if (CALLS[CALLS_LEN - 1].stat != leave.stat) {
		PANIC_AT(leave.loc, "dismatch call at stack and leave call");
	};

	FLowStat * stat = leave.stat;
	u64 const end = rdtsc();
	u64 const delta = end - CALLS[CALLS_LEN - 1].beg;

	stat->cnt += 1;

	stat->total.sum += delta;
	stat->total.min = MIN(stat->total.min, delta);
	// stat->total.max = MAX(stat->total.max, delta);

	if (CALLS_LEN > 1) CALLS[CALLS_LEN - 2].nested += delta;

	stat->nested.sum += CALLS[CALLS_LEN - 1].nested;
	stat->nested.min = MIN(stat->nested.min, CALLS[CALLS_LEN - 1].nested);
	// stat->nested.max = MAX(stat->nested.max, CALLS[CALLS_LEN - 1].nested);

	CALLS_LEN -= 1;
};

void flow_leave_ex(FLowLeave leave) {
	flow_leave_impl(leave);
	// flow_leave_impl((FLowLeave){.stat = &STAT, .loc = FLOW_LOC});
};

// |================================================================================================|
// |> panic                                                                                         |

void NO_RETURN flow_panic(FLowPanic panic) {
	if (IS_PANIC) flow_panic_raw(panic);
	IS_PANIC = true;

	Stream err = stream_output();

	stream_write_arr(&err,
		SL("\n" "\x1B[1;31m" "[panic] " "\x1B[1;37m"), SS(panic.msg), SL("\n"),
		SL("\x1B[1;36m" " --> "), SZ(panic.loc.get), SL("\n"),
		SL("\x1B[1;36m" " --> "), SS(S(FLOW_LOC_LIT)), SL("\n\n"),
	);

	flow_show_calls(&err);

	proc_exit(1);
};

void NO_RETURN flow_panic_raw(FLowPanic panic) {
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
	stream_write_lit(out, "\x1B[1;33m" "Calls stack:" "\x1B[36m" " --> " FLOW_LOC_LIT "\n");

	u64 const idx_width = fmt_u64_shot(CALLS_LEN - 1, &(FmtNumStyle){0}).len;
	u64 tag_width = 0;
	u64 loc_width = 0;
	for (u64 i = 0; i < CALLS_LEN; i += 1) {
		tag_width = MAX(tag_width, str_z_init(CALLS[i].stat->tag.get).len);
		loc_width = MAX(loc_width, str_z_init(CALLS[i].loc.get).len);
	};

	FmtOut fmt = FMT_ON_STACK(KB(4));

	for (u64 i = CALLS_LEN; i > 0; i -= 1) {
		fmt_reset(&fmt);

		fmt_raw(&fmt, "\x1B[34m" "[");
		fmt_u64(&fmt, i - 1, .digits = (u8)idx_width);
		fmt_raw(&fmt, "]" "\x1B[35m" " ");

		fmt_str(&fmt, str_z_init(CALLS[i - 1].stat->tag.get).str, .width = (u32)tag_width);
		fmt_raw(&fmt, "\x1B[36m" " --> ");

		fmt_str(&fmt, str_z_init(CALLS[i - 1].loc.get).str, .width = (u32)loc_width);
		fmt_raw(&fmt, "\n");

		if (i - 1 == 0) fmt_raw(&fmt, "\n");
		stream_write_str(out, fmt_as_str(&fmt));
	};
	return true;
};

b8 flow_show_stats(Stream * out) {
	FmtOut fmt = FMT_ON_STACK(KB(4));

	fmt_raw(&fmt, "\x1B[1;33m" "Calls stats(");
	fmt_u64(&fmt, STATS_LEN);
	fmt_raw(&fmt, "):" "\x1B[36m" " --> " FLOW_LOC_LIT "\n");

	stream_write_str(out, fmt_as_str(&fmt));
	fmt_reset(&fmt);

	u64 const idx_width = fmt_u64_shot(STATS_LEN - 1, &(FmtNumStyle){0}).len;
	u64 tag_width   = 0;
	u64 total_width = 0;
	u64 loc_width   = 0;
	for (u64 i = 0; i < STATS_LEN; i += 1) {
		FLowStat const stat = __start_flow_stats[i];
		tag_width   = MAX(tag_width,   str_z_init(stat.tag.get).len);
		total_width = MAX(total_width, fmt_u64_shot(stat.total.sum - stat.nested.sum, &(FmtNumStyle){0}).len);
		loc_width   = MAX(loc_width,   str_z_init(stat.loc.get).len);
	};

	for (u64 i = 0; i < STATS_LEN; i += 1) {
		FLowStat const stat = __start_flow_stats[i];
		if (stat.cnt == 0) continue;

		fmt_reset(&fmt);

		fmt_raw(&fmt, "\x1B[34m" "[");
		fmt_u64(&fmt, i, .digits = (u8)idx_width);
		fmt_raw(&fmt, "]" "\x1B[35m" " ");

		fmt_str(&fmt, str_z_init(stat.tag.get).str, .width = (u32)tag_width);
		fmt_raw(&fmt, "\x1B[32m" " ");
		fmt_u64(&fmt, stat.total.sum - stat.nested.sum, .width = (u32)total_width, .right = true);
		fmt_raw(&fmt, "\x1B[33m" " ");
		fmt_u64(&fmt, stat.total.sum);
		fmt_raw(&fmt, "\x1B[31m" " ");
		fmt_u64(&fmt, stat.nested.sum);
		fmt_raw(&fmt, "\x1B[36m" " --> ");

		fmt_str(&fmt, str_z_init(stat.loc.get).str);
		fmt_raw(&fmt, "\n");

		stream_write_str(out, fmt_as_str(&fmt));
	};

	stream_write_lit(out, "\n");
	return true;
};

