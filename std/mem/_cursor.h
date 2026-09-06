#ifndef STD_MEM_CURSOR_H
#define STD_MEM_CURSOR_H

#include "std/core.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> [CurLo]: Linear allocator from lower to upper addresses over slice                            |
// |================================================================================================|

/// |---free---|
/// |??????????|
/// ^ pos ---> ^ end
typedef struct {
	uptr pos;
	uptr end;
} CurLo;

#define CUR_LO_EMBED(cur, beg, pos, end) \
	union { \
		CurLo cur; \
		struct { uptr pos; uptr end; }; \
	}

// |================================================================================================|
// |> [CurLo]: constructors                                                                         |

CurLo cur_lo_reg(Reg reg);
CurLo cur_lo_slice(void * ptr, u64 len);

#define CUR_LO_RAW(_pos, _end) ((CurLo){.pos = (_pos), .end = (_end)})
#define CUR_LO_NIL CUR_LO_RAW(0, 0)
#define CUR_LO_ARR(arr) cur_lo_slice((arr), sizeof(arr))

// |================================================================================================|
// |> [CurLo]: helpers                                                                              |

u64 cur_lo_rem(CurLo const * cur);

// |================================================================================================|
// |> [CurLo]: idx <-> ptr                                                                          |

uptr cur_lo_idx(CurLo const * cur, u64  idx);
u64  cur_lo_ptr(CurLo const * cur, uptr ptr);

// |================================================================================================|
// |> [CurLo]: alloc                                                                                |

Reg cur_lo_eval(CurLo const * cur, u64 size, u32 align);
b8  cur_lo_accept(CurLo * cur, Reg reg);
Reg cur_lo_shift(CurLo * cur, u64 size, u32 align);

// |================================================================================================|
// |> [CurLo]: alloc wrappers                                                                       |

#define cur_lo_thing(cur, T) \
	((T*)cur_lo_shift(cur, sizeof(T), alignof(T)))

#define cur_lo_block(cur, T) \
	((T*)cur_lo_shift(cur, offsetof(T, BLOCK_END), alignof(T)))

#define cur_lo_array(cur, count, T) ({ \
	STATIC_ASSERT( \
		(sizeof(T) & (alignof(T) - 1)) == 0 || sizeof(T) <= alignof(T), \
		"to allocate array of item with [size] that [size] must be aligned with [align]" \
	);\
	(T*)cur_lo_shift(cur, count * sizeof(T), alignof(T)); \
})

// |================================================================================================|
// |> [CurLo]: alloc and init                                                                       |

#define _cur_lo_alloc(ID, _cur, _count, _value...) ({ \
	STATIC_ASSERT(IS_TYPE(_cur, CurLo *), "expected mutable CurLo");\
	CurLo        * CAT(cur,  ID) = (_cur); \
	u64              CAT(count, ID) = (_count); \
	typeof(_value) * CAT(ptr, ID) = cur_lo_array(CAT(cur, ID), CAT(count, ID), typeof(_value)); \
	mem_slice_init(CAT(ptr, ID), CAT(count, ID), (_value)); \
	CAT(ptr, ID); \
})

#define cur_lo_alloc(cur, count, value...) _cur_lo_alloc(UNIQ(_cur_lo_alloc_), cur, count, value)


#endif // !STD_MEM_CURSOR_H
