#ifndef STD_MEM_BUMP_H
#define STD_MEM_BUMP_H

#include "std/mem/cur.h"
#include "std/mem/page.h"

// |================================================================================================|
// |> [Bump]: Linear allocator from lower to upper addresses with [RegMan]                          |
// |================================================================================================|

typedef struct {
	uptr beg; CUR_EMBED(cur, pos, end);
	RegMan man;
} Bump;

#define BUMP_EMBED(bump, beg, pos, end, man) \
	union { \
		Bump bump; \
		struct { uptr beg; uptr pos; uptr end; RegMan man; }; \
	}

#define BUMP_SWITCH_CONST(bump, raw, gen) \
	_Generic((bump), \
		BumpR *: (raw), BumpR const *: (raw), \
		Bump *: (gen), Bump const *: (gen) \
	)

#define BUMP_SWITCH_MUT(bump, raw, gen) \
	_Generic((bump), \
		BumpR *: (raw), \
		Bump *: (gen) \
	)

// |================================================================================================|
// |> [Bump]: init                                                                                  |

Bump bump_init(Pages * pages);

// |================================================================================================|
// |> [Bump]: alloc                                                                                 |

Reg bump_raw(Bump * bump, u64 size, u64 align);

// |================================================================================================|
// |> [Bump]: alloc wrappers                                                                        |

#define bump_thing(bump, T) ((T*)bump_raw(bump,              sizeof(T), alignof(T)))
#define bump_block(bump, T) ((T*)bump_raw(bump, offsetof(T, BLOCK_END), alignof(T)))

#define bump_array(bump, count, T) ({ \
	STATIC_ASSERT( \
		(sizeof(T) & (alignof(T) - 1)) == 0 || sizeof(T) <= alignof(T), \
		"to allocate array of item with [size] that [size] must be aligned with [align]" \
	);\
	(T*)bump_raw(bump, (count) * sizeof(T), alignof(T)); \
})

#define _bump_alloc(ID, _bump, _count, _value...) ({ \
	STATIC_ASSERT(IS_TYPE(_bump, Bump *), "expected mutable Bump");\
	Bump           * CAT(bump,  ID) = (_bump); \
	u64              CAT(count, ID) = (_count); \
	typeof(_value) * CAT(ptr, ID) = bump_array(CAT(bump, ID), CAT(count, ID), typeof(_value)); \
	mem_slice_init(CAT(ptr, ID), CAT(count, ID), (_value)); \
	CAT(ptr, ID); \
})

#define bump_alloc(bump, count, value...) _bump_alloc(UNIQ(_bump_alloc_), bump, count, value)

#endif // !STD_MEM_BUMP_H
