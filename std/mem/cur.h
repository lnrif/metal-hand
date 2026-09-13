#ifndef STD_MEM_CURSOR_H
#define STD_MEM_CURSOR_H

#include "std/core.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> [Cur]: Linear allocator from lower to upper addresses over slice                              |
// |================================================================================================|

/// |---free---|
/// |??????????|
/// ^ pos ---> ^ end
typedef struct {
	uptr pos;
	uptr end;
} Cur;

#define CUR_EMBED(cur, pos, end) \
	union { \
		Cur cur; \
		struct { uptr pos; uptr end; }; \
	}

// |================================================================================================|
// |> [Cur]: constructors                                                                           |

Cur cur_from_bounds(uptr beg, uptr end);
Cur cur_from_reg(Reg reg);
Cur cur_from_slice(void * ptr, u64 len);

#define CUR_RAW(_pos, _end) ((Cur){.pos = (_pos), .end = (_end)})
#define CUR_NIL CUR_RAW(0, 0)
#define CUR_FROM_ARR(arr) cur_from_slice((arr), sizeof(arr))

#define CUR_ON_STACK(T, sz) cur_from_slice(ALLOCA((sz) * sizeof(T)), (sz) * sizeof(T))

// |================================================================================================|
// |> [Cur]: marks                                                                                  |

uptr cur_mark(Cur const * cur);
void cur_load(Cur * cur, uptr mark);

// |================================================================================================|
// |> [Cur]: helpers                                                                                |

u64 cur_rem(Cur const * cur);

// |================================================================================================|
// |> [Cur]: shift                                                                                  |

uptr cur_raw(Cur * cur, u64 size, u64 align);

// |================================================================================================|
// |> [Cur]: alloc wrappers                                                                         |

#define cur_thing(cur, T) ((T*)cur_raw(cur, sizeof(T), alignof(T)))
#define cur_block(cur, T) ((T*)cur_raw(cur, offsetof(T, BLOCK_END), alignof(T)))

#define cur_array(cur, count, T) ({ \
	STATIC_ASSERT( \
		(sizeof(T) & (alignof(T) - 1)) == 0 || sizeof(T) <= alignof(T), \
		"to allocate array of item with [size] that [size] must be aligned with [align]" \
	); \
	(T*)cur_raw(cur, (count) * sizeof(T), alignof(T)); \
})

// |================================================================================================|
// |> [Cur]: alloc and init                                                                         |

#define _cur_alloc(ID, _cur, _count, _value...) ({ \
	STATIC_ASSERT(IS_TYPE(_cur, Cur *), "expected mutable Cur");\
	Cur *            CAT(cur,  ID) = (_cur); \
	u64              CAT(count, ID) = (_count); \
	typeof(_value) * CAT(ptr, ID) = cur_array(CAT(cur, ID), CAT(count, ID), typeof(_value)); \
	mem_slice_init(CAT(ptr, ID), CAT(count, ID), (_value)); \
	CAT(ptr, ID); \
})

#define cur_alloc(cur, count, value...) _cur_alloc(UNIQ(_cur_alloc_), cur, count, value)

#endif // !STD_MEM_CURSOR_H
