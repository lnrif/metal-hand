#include "std/mem/bump.h"
#include "std/flow/core.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> [Bump]: init                                                                                  |

Bump bump_init(Pages * pages) {
	return (Bump){.beg = 0, .cur = CUR_NIL, .man = pages_pinned(pages) };
};

// |================================================================================================|
// |> [Bump]: alloc                                                                                 |

// TODO: optimize with [Pages]
Reg bump_raw(Bump * bump, u64 size, u64 align) {
	ASSERT(bump != 0, "invalid [bump]");
	ASSERT(ALIGN_IS_SANE(align), "invalid [aling]");
	ASSERT(bump->beg <= bump->pos && bump->pos <= bump->end, "invalid [bump] invariant");

	uptr ptr; if (ADD_OVER_DBG(bump->pos, align - 1, &ptr)) return REG_NIL;
	ptr &= ~(align - 1);
	uptr pos; if (ADD_OVER_DBG(ptr, size, &pos)) return REG_NIL;

	if (pos <= bump->end) return REG(ptr, pos - ptr);

	Reg const prev = (Reg){.ptr = bump->beg, .len = bump->end - bump->beg};
	Reg const next = reg_upd(
		bump->man,
		REG_UPD_ARR(
			prev.raw, prev.len,
			prev.len + (pos - bump->end), REG_DIR_UP
		)
	);

	if (next.ptr == 0) return REG_NIL;

	Reg reg = (Reg){
		.ptr = next.ptr + (ptr - bump->beg),
		.len = size,
	};

	bump->pos = next.ptr + (pos - bump->beg);
	bump->beg = next.ptr;
	bump->end = next.ptr + next.len;

	return reg;
};

