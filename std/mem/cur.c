#include "std/mem/cur.h"
#include "std/mem/core.h"
#include "std/flow/core.h"

// |================================================================================================|
// |> [Cur]: Linear allocator from lower to upper addresses over slice                              |
// |================================================================================================|

// |================================================================================================|
// |> [Cur]: constructors                                                                           |

Cur cur_from_bounds(uptr beg, uptr end) {
	ASSERT(beg <= end, "invalid bounds, expected that [beg] <= [end]");
	ASSERT(beg != 0 || (beg == 0 && end == 0), "invalid [bump]");
	return CUR_RAW(beg, end);
};

Cur cur_from_reg(Reg reg) { return cur_from_bounds(reg.ptr, ADD(uptr, reg.ptr, reg.len)); };
Cur cur_from_slice(void * ptr, u64 len) { return cur_from_reg(REG_SLICE(ptr, len)); };

// |================================================================================================|
// |> [Cur]: marks                                                                                  |

uptr cur_mark(Cur const * cur) { return cur->pos; };
void cur_load(Cur * cur, uptr mark) { cur->pos = mark; };

// |================================================================================================|
// |> [Cur]: helpers                                                                                |

u64 cur_rem(Cur const * cur) { return cur->end - cur->pos; };

// |================================================================================================|
// |> [Cur]: shift                                                                                  |

uptr cur_raw(Cur * cur, u64 size, u64 align) {
	ASSERT_PTR(cur); ASSERT_ALLOC(align, size);
	ASSERT(cur->pos <= cur->end, "invalid [cur] invariant");

	uptr const ptr = mem_align_up(cur->pos, align);
	if (ptr == U64_MAX) return PTR_NIL;

	uptr pos; if (ADD_OVER(ptr, size, &pos)) return PTR_NIL;
	if (pos > cur->end) return PTR_NIL;

	cur->pos = pos;
	return ptr;
};

// |================================================================================================|
// |> [Cur]: alloc wrappers                                                                         |

// |================================================================================================|
// |> [Cur]: alloc and init                                                                         |
