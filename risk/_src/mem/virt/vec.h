#ifndef RK_MEM_VIRT_VEC_H
#define RK_MEM_VIRT_VEC_H

#include "mem/virt/page.h"
#include "mem/quota.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// common vec api

#define VEC_INDEX_ASSERT(vec, i) ASSERT(i < (vec)->len, "index (%llu) out of bounds", (usz)(i))
#define vec__index_throw(vec, i) ({VEC_INDEX_ASSERT(vec, i); i;})
#define VEC_INDEX_UNSAFE(vec, i) ((vec)->ptr[i])
#define VEC_INDEX(vec, i)        ((vec)->ptr[vec__index_throw(vec, i)])

#define vec__pop_success(vec, dst) ({(vec)->len -= 1; *(dst) = (vec)->ptr[(vec)->len + 1]; TRUE;})
#define VEC_POP(vec, dst) ((vec)->len > 0 ? vec__pop_success(vec, dst) : FALSE)

#define vec_iter_typed(T, vec, it, i, body...) \
	for (usz i = 0; i < (vec)->len; i += 1) { \
		T * it = &(vec)->ptr[i]; \
		body; \
	}

#define vec_iter_rev_typed(T, vec, it, i, body...) \
	do { \
		usz i = (vec)->len; \
		while (i != 0) { \
			i -= 1; T * it = &(vec)->ptr[i]; \
			body; \
		}; \
	} while (0)

#define vec_iter(vec, it, i, body...)     vec_iter_typed(typeof(*(vec)->ptr), vec, it, i, body)
#define vec_iter_rev(vec, it, i, body...) vec_iter_rev_typed(typeof(*(vec)->ptr), vec, it, i, body)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// slice vec

#define SLICE_VEC_FIELDS(T) \
	T * ptr; usz len; usz cap \

#define SLICE_VEC_EMBED(T) \
	union \
		struct { SLICE_VEC_FIELDS(T); }; \
		SliceVec slice_vec; \
	}

typedef struct {
	uptr ptr;
	usz len;
	usz cap;
} SliceVec;

SliceVec slice_vec_init_ex(void * ptr, usz len, usz size, usz align);

#define slice_vec_type(V) typeof(*((V*)NULL)->ptr)

#define slice_vec_init_typed(T, ptr, len) slice_vec_init_ex(ptr, len, sizeof(T), alignof(T))
#define slice_vec_init(V, ptr, len) ((V){.slice_vec = slice_vec_init_typed(slice_vec_type(V), ptr, len)})
#define slice_vec_from_array(V, buf) slice_vec_init(typeof(V), buf, LEN(buf))

#define slice_vec__push_success(vec, value...) ({(vec)->ptr[(vec)->len] = value; (vec)->len += 1; TRUE;})
#define SLICE_VEC_PUSH(vec, value...) ((vec)->len < (vec)->cap ? slice_vec__push_success(vec, value) : FALSE)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// virt vec

#define VIRT_VEC_FIELDS(T) \
	SLICE_VEC_FIELDS(T); \
	struct { uptr pos; uptr end; usz step; } page; \
	Quota * quota

#define VIRT_VEC_EMBED(T) \
	union { \
		struct { VIRT_VEC_FIELDS(T); }; \
		VirtVec virt_vec; \
	}

// beg|-----------|--------|pos-----------|end
//    |----len----|        |              |
//    |---------cap--------|              |
// beg = ptr
// end = ptr + len * item.size
typedef struct {
	uptr ptr; usz len; usz cap;
	// virtual memory (if step != 0)
	struct { uptr pos; uptr end; usz step; } page;
	Quota * quota;
} VirtVec;

typedef struct {
	usz step;
	usz size;
} VirtVecOpt;

VirtVec virt_vec_init_ex(Memory * mem, usz item_size, usz item_align, VirtVecOpt opt);

#define VIRT_VEC_TYPE(V) typeof(*((V*)NULL)->ptr)

#define virt_vec_init(V, mem, ...) \
	virt_vec_init_ex(mem, sizeof(VIRT_VEC_TYPE(V)), alignof(VIRT_VEC_TYPE(V)), (VirtVecOpt){__VA_ARGS__})

bool virt_vec_grow_one_impl(VirtVec * vec, usz size, usz align);
bool virt_vec_grow_one_ex(VirtVec * vec, usz size, usz align);
#define virt_vec_grow_one(T, vec) virt_vec_grow_one_ex(vec, sizeof(T), alignof(T))

#define virt_vec__push_success(vec, value...) ({(vec)->ptr[(vec)->len] = value; (vec)->len += 1; TRUE;})
#define VIRT_VEC_PUSH(vec, value...) \
	(virt_vec_grow_one(typeof(*(vec)->ptr), &(vec)->virt_vec) ? virt_vec__push_success(vec, value) : FALSE)

#endif // RK_MEM_VIRT_VEC_H
