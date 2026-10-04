#include "std/mem/bump.h"
#include "std/flow/core.h"
#include "std/mem/cur.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> [Bump]: init                                                                                  |

Bump bump_init(Pages * pages) {
	return (Bump){.beg = 0, .cur = CUR_NIL, .man = pages_pinned(pages) };
};

// |================================================================================================|
// |> [Bump]: alloc                                                                                 |

uptr bump_raw(Bump * bump, u64 size, u64 align) {
	ASSERT(bump != 0, "invalid [bump]");
	ASSERT(ALIGN_IS_SANE(align), "invalid [aling]");
	ASSERT(bump->beg <= bump->pos && bump->pos <= bump->end, "invalid [bump] invariant");

	uptr ptr = cur_raw(&bump->cur, size, align);
	if (ptr != 0) return ptr;

	Reg const prev = (Reg){.ptr = bump->beg, .len = bump->end - bump->beg};
	Reg const next = reg_upd(
		bump->man,
		REG_UPD_ARR(
			prev.raw, prev.len,
			prev.len + size + align, REG_DIR_UP
		)
	);

	if (next.ptr == 0) return 0;
	bump->pos = next.ptr + (bump->pos - bump->beg);
	bump->beg = next.ptr;
	bump->end = next.ptr + next.len;

	return cur_raw(&bump->cur, size, align);
};

