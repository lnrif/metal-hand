
#ifndef RK_MEM_REAL_VEC_H
#define RK_MEM_REAL_VEC_H

#include "core/types.h"
#include "trc/assert.h"

typedef struct { uptr ptr; usz len; } Mem;
typedef Mem MemCall(void * ctx, Mem mem, usz align, usz new, bool copy);

typedef struct { void * ctx; MemCall * call; } Allocator;

#define VEC_FIELDS(T) \
	T ptr; usz len; usz cap; \
	Allocator allocator

typedef struct { VEC_FIELDS(uptr); } Vec;

#define DEF_VEC(T, Name) \
	typedef struct { VEC_FIELDS(T*); } Name;

bool vec_reserve_raw(Vec * vec, usz count, usz align, usz size);

bool vec_reserve_raw(Vec * vec, usz count, usz align, usz size) {
	usz new_cap = ADD(CALL, vec->len, count);
	if (new_cap <= vec->cap) return TRUE;

	uptr new_ptr = vec->allocator.call(
		vec->allocator.ctx,
		vec->ptr,
		align,
		MUL(CALL, size, vec->cap),
		MUL(CALL, size, new_cap)
	);

	if (new_ptr == 0) return FALSE;
	vec->ptr = new_ptr;
	vec->cap = new_cap;
	return TRUE;
};

#define vec_reserve(vec, count) \
	vec_reserve_raw(vec, count, )

#define vec_push(vec, value...) \
	({ \
		typeof(vec)   _vec   = (vec); \
		typeof(value) _value = (value); \
		if (_vec.len >= _vec.cap) { \
			\
		}; \
		vec.allocator\
	})

// usz bump_len_ex(Bump const * bump, usz size);
#define bump_len(bump, T) bump_len_ex(bump, sizeof(T))

#define bump_index(bump, T, index) \
	(((T const *)(bump)->beg)[({ \
		STATIC_ASSERT(IS_TYPE(bump, Bump *) || IS_TYPE(bump, Bump const *), "expected Bump");\
		usz _bump_index = (index); \
		ASSERT_DEBUG_AT(CALL, _bump_index < bump_len(bump, T), "index out of bounds"); \
		_bump_index; \
	})])

#define bump_index_mut(bump, T, index) \
	(((T*)(bump)->beg)[({ \
		STATIC_ASSERT(IS_TYPE(bump, Bump *), "expected mutable Bump");\
		usz _bump_index = (index); \
		ASSERT_DEBUG_AT(CALL, _bump_index < bump_len(bump, T), "index out of bounds"); \
		_bump_index; \
	})])

#endif // !RK_MEM_REAL_VEC_H
