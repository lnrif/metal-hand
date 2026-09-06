#include "std/mem/cursor.h"

// |================================================================================================|
// |> [CurLo]: Linear allocator from lower to upper addresses over slice                            |
// |================================================================================================|

// |================================================================================================|
// |> [CurLo]: constructors                                                                         |

CurLo cur_lo_init(uptr pos, uptr end) {
	if (pos > end || pos == 0 || end == 0) return CUR_LO_NIL;
	return (CurLo){.pos = pos, .end = end};
};

CurLo cur_lo_reg(Reg reg) { return cur_lo_slice(reg.any, reg.len); };
CurLo cur_lo_slice(void * ptr, u64 len) { return CUR_LO((uptr)ptr, (uptr)ptr + len); };

// |================================================================================================|
// |> [CurLo]: helpers                                                                              |

u64 cur_lo_rem(CurLo const * cur) {
	return cur->end - cur->pos;
};

// |================================================================================================|
// |> [CurLo]: idx <-> ptr                                                                          |

uptr cur_lo_idx(CurLo const * cur, u64 idx) {
	return 
};


u64  cur_lo_ptr(CurLo const * cur, uptr ptr);

// |================================================================================================|
// |> [CurLo]: alloc                                                                                |

Alloc cur_lo_eval(CurLo const * cur, u64 size, u32 align);
b8    cur_lo_accept(CurLo * cur, Alloc alloc);
Alloc cur_lo_shift(CurLo * cur, u64 size, u32 align);


