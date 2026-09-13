#ifndef RK_MEM_BUMP_ALLOC_H
#define RK_MEM_BUMP_ALLOC_H

#include "std/core.h"
#include "std/mem/reg.h"

// |================================================================================================|
// |> [BumpR]: Linear allocator from lower to upper addresses over slice                            |
// |================================================================================================|

///   |---use---|---free---|
///   |xxxxxxxxx???????????|
///   ^ beg     ^ pos ---> ^ end
typedef struct {
	uptr beg;
	uptr pos;
	uptr end;
} BumpR;

#define BUMP_R_EMBED(bump, beg, pos, end) \
	union { \
		BumpR bump; \
		struct { uptr beg; uptr pos; uptr end; }; \
	}

// |================================================================================================|
// |> [BumpR]: constructors                                                                         |

#define BUMP_R_NIL ((BumpR){.beg = 0, .pos = 0, .end = 0})

BumpR bump_r_init(uptr beg, uptr end);
BumpR bump_r_from_slice(void * ptr, u64 len);

#define bump_r_from_array(arr) \
	((BumpR){ \
		.beg = (uptr)(arr), \
		.pos = (uptr)(arr), \
		.end = (uptr)(arr) + sizeof(arr), \
	}) // (uptr)(arr) + sizeof(arr) cannot overflow

#define BUMP_R_ON_STACK(size) bump_r_from_slice(ALLOCA(size), size)

// |================================================================================================|
// |> [BumpR]: marks                                                                                |

uptr bump_r_mark(BumpR const * bump);
void bump_r_load(BumpR * bump, uptr mark);
void bump_r_reset(BumpR * bump);

// |================================================================================================|
// |> [BumpR]: helpers                                                                              |

u64 bump_r_rem(BumpR const * bump);
u64 bump_r_len(BumpR const * bump);
u64 bump_r_cap(BumpR const * bump);

// |================================================================================================|
// |> [BumpR]: idx <-> ptr                                                                          |

uptr bump_r_idx(BumpR const * bump, u64  idx);
u64  bump_r_ptr(BumpR const * bump, uptr ptr);

// |================================================================================================|
// |> [BumpR]: alloc                                                                                |

Reg bump_r_eval(BumpR const * bump, u64 size, u32 align);
b8  bump_r_accept(BumpR * bump, Reg alloc);
Reg bump_r_shift(BumpR * bump, u64 size, u32 align);

// |================================================================================================|
// |> [BumpR]: alloc wrappers                                                                       |

#define bump_r_thing(bump, T) \
	((T*)bump_r_shift(bump, sizeof(T), alignof(T)))

#define bump_r_block(bump, T) \
	((T*)bump_r_shift(bump, offsetof(T, BLOCK_END), alignof(T)))

#define bump_r_array(bump, count, T) ({ \
	STATIC_ASSERT( \
		(sizeof(T) & (alignof(T) - 1)) == 0 || sizeof(T) <= alignof(T), \
		"to allocate array of item with [size] that [size] must be aligned with [align]" \
	);\
	(T*)bump_r_shift(bump, count * sizeof(T), alignof(T)); \
})

// |================================================================================================|
// |> [BumpR]: alloc and init                                                                       |

#define _bump_r_alloc(ID, _bump, _count, _value...) ({ \
	STATIC_ASSERT(IS_TYPE(_bump, BumpR *), "expected mutable BumpR");\
	BumpR        *   CAT(bump,  ID) = (_bump); \
	u64              CAT(count, ID) = (_count); \
	typeof(_value) * CAT(ptr, ID) = bump_r_array(CAT(bump, ID), CAT(count, ID), typeof(_value)); \
	mem_slice_init(CAT(ptr, ID), CAT(count, ID), (_value)); \
	CAT(ptr, ID); \
})

#define bump_r_alloc(bump, count, value...) _bump_r_alloc(UNIQ(_bump_r_alloc_), bump, count, value)

// |================================================================================================|
// |> [Bump]: Linear allocator from lower to upper addresses with [RegMan]                          |
// |================================================================================================|

typedef struct {
	BUMP_R_EMBED(raw, beg, pos, end);
	RegMan man; // NOTE: for now using 2x grow (maybe add scale)
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
// |> [Bump]: alloc and init                                                                        |

#define bump_init(_man)     ((Bump){.raw = BUMP_R_NIL, .man = (_man)})
#define bump_from_raw(_raw) ((Bump){.raw = (_raw), .man = ALLOC_CTX_NIL})

// |================================================================================================|
// |> [Bump]: alloc                                                                                 |

Reg bump_gen_shift(Bump * bump, u64 size, u32 align);

// |================================================================================================|
// |> [Bump]: alloc wrappers                                                                        |

#define bump_gen_thing(bump, T) ((T*)bump_gen_shift(bump,              sizeof(T), alignof(T)))
#define bump_gen_block(bump, T) ((T*)bump_gen_shift(bump, offsetof(T, BLOCK_END), alignof(T)))

#define bump_gen_array(bump, count, T) ({ \
	STATIC_ASSERT( \
		(sizeof(T) & (alignof(T) - 1)) == 0 || sizeof(T) <= alignof(T), \
		"to allocate array of item with [size] that [size] must be aligned with [align]" \
	);\
	(T*)bump_gen_shift(bump, (count) * sizeof(T), alignof(T)); \
})

#define _bump_gen_alloc(ID, _bump, _count, _value...) ({ \
	STATIC_ASSERT(IS_TYPE(_bump, Bump *), "expected mutable Bump");\
	Bump           * CAT(bump,  ID) = (_bump); \
	u64              CAT(count, ID) = (_count); \
	typeof(_value) * CAT(ptr, ID) = bump_gen_array(CAT(bump, ID), CAT(count, ID), typeof(_value)); \
	mem_slice_init(CAT(ptr, ID), CAT(count, ID), (_value)); \
	CAT(ptr, ID); \
})

#define bump_gen_alloc(bump, count, value...) _bump_gen_alloc(UNIQ(_bump_gen_alloc_), bump, count, value)

// |================================================================================================|
// |> [Str]                                                                                         |

#include "std/str/str.h"

Str bump_r_as_str(BumpR const * bump);
#define bump_gen_as_str(bump) bump_r_as_str(&(bump)->raw)

#define bump_as_str(bump) BUMP_SWITCH_CONST((bump), \
	bump_r_as_str(AS(BumpR, bump)), \
	bump_gen_as_str(&AS(Bump, bump)->raw) \
)

// |================================================================================================|
// |> [Bump]: shift                                                                                 |

#define bump_shift(bump, size, align...) BUMP_SWITCH_MUT((bump), \
	bump_r_shift(AS(BumpR, bump), size, align), \
	bump_gen_shift(&AS(Bump, bump)->raw, size, align) \
)

#define bump_thing(bump, T) BUMP_SWITCH_MUT((bump), \
	bump_r_thing(AS(BumpR, bump), T), \
	bump_gen_thing(&AS(Bump, bump)->raw, T) \
)

#define bump_block(bump, T) BUMP_SWITCH_MUT((bump), \
	bump_r_block(AS(BumpR, bump), T), \
	bump_gen_block(&AS(Bump, bump)->raw, T) \
)

#define bump_array(bump, count, T) BUMP_SWITCH_MUT((bump), \
	bump_r_array(AS(BumpR, bump), count, T), \
	bump_gen_array(&AS(Bump, bump)->raw, count, T) \
)

#define bump_alloc(bump, count, value...) BUMP_SWITCH_MUT((bump), \
	bump_r_alloc(AS(BumpR, bump), count, value), \
	bump_gen_alloc(&AS(Bump, bump)->raw, count, value) \
)

// |================================================================================================|
// |> [Bump]: marks                                                                                 |

#define bump_mark(bump...) BUMP_SWITCH_CONST((bump), \
	bump_r_mark(AS(BumpR, bump)), \
	bump_r_mark(&AS(Bump, bump)->raw) \
)

#define bump_load(bump, mark...) BUMP_SWITCH_MUT((bump), \
	bump_r_load(AS(BumpR, bump), mark), \
	bump_r_load(&AS(Bump, bump)->raw, mark) \
)

#define bump_reset(bump, mark...) BUMP_SWITCH_MUT((bump), \
	bump_r_reset(AS(BumpR, bump), mark), \
	bump_r_reset(&AS(Bump, bump)->raw, mark) \
)

// |================================================================================================|
// |> [Bump]: helpers                                                                               |

#define bump_beg(bump) ((void*)(bump)->beg)
#define bump_pos(bump) ((void*)(bump)->pos)
#define bump_end(bump) ((void*)(bump)->end)

#define bump_rem(bump...) BUMP_SWITCH_CONST((bump), \
	bump_r_rem(AS(BumpR, bump)), \
	bump_r_rem(&AS(Bump, bump)->raw) \
)

#define bump_len(bump...) BUMP_SWITCH_CONST((bump), \
	bump_r_len(AS(BumpR, bump)), \
	bump_r_len(&AS(Bump, bump)->raw) \
)

#define bump_cap(bump...) BUMP_SWITCH_CONST((bump), \
	bump_r_cap(AS(BumpR, bump)), \
	bump_r_cap(&AS(Bump, bump)->raw) \
)

// |================================================================================================|
// |> [Bump]: idx <-> ptr                                                                           |

#define bump_idx(bump, idx...) BUMP_SWITCH_CONST((bump), \
	bump_r_idx(AS(BumpR, bump), idx), \
	bump_r_idx(&AS(Bump, bump)->raw, idx) \
)

#define bump_ptr(bump, ptr...) BUMP_SWITCH_CONST((bump), \
	bump_r_ptr(AS(BumpR, bump), ptr), \
	bump_r_ptr(&AS(Bump, bump)->raw, ptr) \
)

#endif // !RK_MEM_BUMP_ALLOC_H
