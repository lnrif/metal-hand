#include "std/mem/bump.h"
#include "std/flow/core.h"
#include "std/mem/reg.h"

#if !defined(DEBUG)
	#undef ASSERT
	#define ASSERT(cond, fmt...) UNUSED(cond)
#endif

// |================================================================================================|
// |> [BumpR]: constructors                                                                       |

BumpR bump_r_init(uptr beg, uptr end) {
	ASSERT(beg <= end, "invalid bounds, expected that [beg] <= [end]");
	ASSERT(beg != 0 || (beg == 0 && end == 0), "invalid [bump]");
	return (BumpR){.beg = beg, .pos = beg, .end = end};
};

BumpR bump_r_from_slice(void * ptr, u64 len) {
	ASSERT(ptr != 0 || (ptr == 0 && len == 0), "invalid [bump]");
	uptr beg = (uptr)ptr; uptr end = 0;
	ASSERT(!ADD_OVER_DBG(beg, len, &end), "overflow");
	return bump_r_init(beg, end);
};

// |================================================================================================|
// |> [BumpR]: marks                                                                              |

uptr bump_r_mark(BumpR const * bump) {
	ASSERT_PTR(bump);
	return bump->pos;
};

void bump_r_load(BumpR * bump, uptr mark) {
	ASSERT_PTR(bump);
	ASSERT(bump->beg <= mark && mark <= bump->end, "[mark] is out of [bump]");
	if (mark <= bump->pos) bump->pos = mark;
};

void bump_r_reset(BumpR * bump) {
	ASSERT_PTR(bump);
	bump->pos = bump->beg;
};

// |================================================================================================|
// |> [BumpR]: helpers                                                                            |

u64 bump_r_rem(BumpR const * bump) {
	ASSERT_PTR(bump);
	return bump->end - bump->pos;
};

u64 bump_r_len(BumpR const * bump) {
	ASSERT_PTR(bump);
	return bump->pos - bump->beg;
};

u64 bump_r_cap(BumpR const * bump) {
	ASSERT_PTR(bump);
	return bump->end - bump->beg;
};

// |================================================================================================|
// |> [BumpR]: idx <-> ptr                                                                        |

uptr bump_r_idx(BumpR const * bump, u64 idx) {
	uptr ptr; return (!ADD_OVER_DBG(bump->beg, idx, &ptr) && ptr < bump->pos) ? ptr : 0;
};

u64 bump_r_ptr(BumpR const * bump, uptr ptr) {
#ifdef DEBUG
	if (ptr < bump->beg || bump->pos <= ptr) return U64_MAX;
#endif
	return ptr - bump->beg;
};

// |================================================================================================|
// |> [BumpR]: alloc                                                                              |

Reg bump_r_eval(BumpR const * bump, u64 size, u32 align) {
	ASSERT(bump != 0, "invalid [bump]");
	ASSERT(ALIGN_IS_SANE(align), "invalid [aling]");
	ASSERT(bump->beg <= bump->pos && bump->pos <= bump->end, "invalid [bump] invariant");

	uptr ptr; if (ADD_OVER_DBG(bump->pos, align - 1, &ptr)) return REG_NIL;
	ptr &= ~(align - 1);
	uptr pos; if (ADD_OVER_DBG(ptr, size, &pos)) return REG_NIL;

	return (Reg){.ptr = ptr, .len = pos - ptr};
};

b8 bump_r_accept(BumpR * bump, Reg reg) {
	if (reg.ptr == 0) return false; // overflow

	uptr const pos = (uptr)reg.ptr + reg.len;
	if (pos > bump->end) return false;

	bump->pos = pos;
	return true;
};

Reg bump_r_shift(BumpR * bump, u64 size, u32 align) {
	Reg reg = bump_r_eval(bump, size, align);
	if (bump_r_accept(bump, reg)) return reg;
	return REG_NIL;
};

// |================================================================================================|
// |> [Bump]: alloc                                                                                 |

// Reg bump_gen_shift(Bump * bump, u64 size, u32 align) {
// 	Reg reg = bump_r_shift(&bump->raw, size, align);
// 	if (reg.ptr != 0) return reg;
//
// 	Reg    const prev = (Reg){.ptr = bump->beg, .len = bump_cap(bump)};
// 	RegReq const req  = REG_REQ(0, 0, 0);
// 	Reg    const next = reg_upd(bump->man, REG_UPD(prev, req));
//
// 	Reg next = alloc_call(
// 		bump->ctx,
// 		,
// 		(RegArgs){
// 			.len = ALLOC_END(&reg) - bump->beg,
// 			.align = bump->beg == 0 ? align : 1 << CTZ(bump->beg),
// 		}
// 	);
// 	if (next.ptr == 0) return REG_NIL;
//
// 	reg = (Reg){
// 		.ptr = next.ptr + (reg.ptr - bump->raw.beg),
// 		.len = reg.len,
// 	};
//
// 	bump->raw = (BumpR){
// 		.beg = next.ptr,
// 		.pos = ALLOC_END(&reg),
// 		.end = next.ptr + next.len,
// 	};
//
// 	return reg;
// };

// |================================================================================================|
// |> [Str]

Str bump_r_as_str(BumpR const * bump) {
	return (Str){
		.ptr = bump_beg(bump),
		.len = bump_len(bump),
	};
};

