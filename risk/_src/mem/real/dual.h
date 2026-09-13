#ifndef RK_MEM_REAL_DUAL_H
#define RK_MEM_REAL_DUAL_H

#include "mem/real/bump.h"
#include "mem/real/stack.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// dual

#define DUAL_FIELDS(bump, stack) \
	struct { \
		uptr beg; \
		uptr bump_pos; \
		uptr stack_pos; \
		uptr end; \
	}; \
	struct { \
		Bump bump; \
		uptr _beg; \
	}; \
	struct { \
		uptr _end; \
		Stack stack; \
	}

/// |----------|--------------bump------------------|
/// ^ beg      ^ bp -> -> -> -> grow -> -> -> -> -> ^ sp
///            |---------------------stack----------|---------|
///            ^ pl <- <- <- <- grow <- <- <- <- <- ^ sp      ^ end
typedef union { DUAL_FIELDS(bump, stack); } Dual;

#define DUAL_EMBED(stack, up, down) \
	union { \
		struct { DUAL_FIELDS(up, down); }; \
		Dual dual; \
	}

STATIC_ASSERT(sizeof(Bump)  == sizeof(uptr) * 3, "[Bump] size must be exactly 3 pointers");
STATIC_ASSERT(sizeof(Stack) == sizeof(uptr) * 3, "[Stack] size must be exactly 3 pointers");
STATIC_ASSERT(sizeof(Dual)  == sizeof(uptr) * 4, "[Dual] size must be exactly 4 pointers");

////////////////////////////////
// constructors

Dual dual_init(void * ptr, usz len);

#define dual_from_array(arr) \
	((Dual){ \
		.beg      = (uptr)(arr), \
		.pos_up   = (uptr)(arr), \
		.pos = (uptr)(arr) + sizeof(arr), \
		.hgih     = (uptr)(arr) + sizeof(arr), \
	}) // (uptr)(arr) + sizeof(arr) cannot overflow

////////////////////////////////
// temporary

typedef struct { Dual * get; uptr bump_mark; uptr stack_mark; } DualTemp;

DualTemp dual_temp(Dual * stack);
void     dual_finish(DualTemp temp);

////////////////////////////////
// mark & reset

void dual_reset(Dual * stack);

#endif // !RK_MEM_REAL_DUAL_H
