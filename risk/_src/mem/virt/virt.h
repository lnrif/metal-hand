#ifndef RK_MEM_VIRT_VIRT_H
#define RK_MEM_VIRT_VIRT_H

#include "mem/real/bump.h"
#include "mem/real/dual.h"
#include "mem/virt/page.h"
#include "mem/virt/vec.h"
#include "mem/quota.h"

typedef struct {
	usz step;
	usz size;
	// usz shift;
} VirtInitOpts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// virt up

typedef struct {
	// allocate like all memory is commited
	BUMP_EMBED(bump, beg, pos, end);
	// not comitted region between arena and stack.
	struct { uptr pos; usz step; } page;
	Quota * quota;
} VirtBump;

#define virt_bump_init(mem, ...) virt_bump_init_ex(mem, (VirtInitOpts){ __VA_ARGS__ })

VirtBump virt_bump_init_ex(Memory * memory, VirtInitOpts opt);

void   virt_bump_shrink(VirtBump * virt);
void   virt_bump_decommit(VirtBump * virt);
bool   virt_bump_commit_impl(VirtBump * virt);
bool   virt_bump_commit(VirtBump * virt);

#define virt_bump_reset(virt) bump_reset(&(virt)->bump)

#define virt_bump_thing(virt, T)                bump_thing(&(virt)->bump, T)
#define virt_bump_block(virt, T)                bump_block(&(virt)->bump, T)
#define virt_bump_array(virt, count, T)         bump_array(&(virt)->bump, count, T)
#define virt_bump_raw(virt, count, size, align) bump_raw(&(virt)->bump, count, size, align)

#define virt_bump_mark(virt)       bump_mark(&(virt)->bump)
#define virt_bump_load(virt, mark) bump_load(&(virt)->bump, mark)

#define _urm_virt_bump_take(ID, _virt, _count, T) ({ \
	VirtBump * CAT(virt, ID)   = (_virt); \
	usz        CAT(count, ID) = (_count); \
	T * CAT(ptr, ID) = virt_bump_array(CAT(virt, ID), T, CAT(count, ID)); \
	if (CAT(ptr, ID) == NULL || !virt_bump_commit(CAT(virt, ID))) CAT(ptr, ID) = NULL; \
	CAT(ptr, ID); \
})

#define _urm_virt_bump_alloc(ID, _virt, _count, _value...) ({ \
	VirtBump *     CAT(virt, ID)   = (_virt); \
	usz            CAT(count, ID) = (_count); \
	typeof(_value) CAT(value, ID) = (_value); \
	\
	typeof(_value) * CAT(ptr, ID) = virt_bump_array(CAT(virt, ID), CAT(count, ID), typeof(_value)); \
	\
	if (CAT(ptr, ID) != NULL && virt_bump_commit(CAT(virt, ID))) { \
		for (usz CAT(i, ID) = 0; CAT(i, ID) < CAT(count, ID); CAT(i, ID) += 1) { \
			CAT(ptr, ID)[CAT(i, ID)] = CAT(value, ID); \
		}; \
	} else { \
		CAT(ptr, ID) = NULL; \
	}; \
	\
	CAT(ptr, ID); \
})

#define virt_bump_alloc(virt, count, value...) _urm_virt_bump_alloc(UNIQ(_urm_virt_bump_alloc_), virt, count, value)
#define virt_bump_take(virt, count, T)         _urm_virt_bump_take(UNIQ(_urm_virt_bump_take_), virt, count, T)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// virt down

typedef struct {
	// allocate like all memory is commited
	STACK_EMBED(stack, beg, pos, end);
	// not comitted region between arena and stack.
	struct {
		uptr pos;
		// if [step] == 0 then working on stacks, otherwise we have pages.
		usz step;
	} page;
	Quota * quota;
} VirtStack;

typedef StackTemp VirtTemp;
#define virt_stack_temp(virt) stack_temp(&(virt)->stack)
#define virt_stack_finish stack_finish

#define virt_stack_init(mem, ...) virt_stack_init_ex(mem, (VirtInitOpts){ __VA_ARGS__ })

VirtStack virt_stack_init_ex(Memory * mem, VirtInitOpts opt);

void virt_stack_shrink(VirtStack * virt);
void virt_stack_decommit(VirtStack * virt);
bool virt_stack_commit_impl(VirtStack * virt);
bool virt_stack_commit(VirtStack * virt);

uptr virt_stack_mark(VirtStack const * virt);
void virt_stack_load(VirtStack       * virt, uptr mark);

bool virt_stack_split_virt(VirtStack * src, VirtStack * dst, usz size, usz step, Quota * quota);
// bool virt_stack_split(VirtStack * src, void * dst, usz size);

#define virt_stack_split_vec(src, dst, size, step, quota) \
	virt_stack_split_vec_ex( \
		src, &(dst)->virt_stack_vec, \
		size, \
			 sizeof(stack_vec_type(typeof(*(dst)))), \
			alignof(stack_vec_type(typeof(*(dst)))), \
		step, quota \
	)

bool virt_stack_split_vec_ex(
	VirtStack * src, VirtVec * dst,
	usz size, usz item_size, usz item_align,
	usz step, Quota * quota
);

#define virt_stack_reset(virt) stack_reset(&(virt)->stack)

#define virt_stack_thing(virt, T)                stack_thing(&(virt)->stack, T)
#define virt_stack_block(virt, T)                stack_block(&(virt)->stack, T)
#define virt_stack_array(virt, T, count)         stack_array(&(virt)->stack, T, count)
#define virt_stack_raw(virt, count, size, align) stack_raw(&(virt)->stack, count, size, align)

#define _urm_virt_stack_alloc(ID, _virt, _count, _value...) ({ \
	VirtStack *      CAT(virt, ID)   = (_virt); \
	usz              CAT(count, ID) = (_count); \
	typeof(_value) CAT(value, ID) = (_value); \
	\
	typeof(_value) * CAT(ptr, ID) = virt_stack_array(CAT(virt, ID), CAT(count, ID), typeof(_value)); \
	\
	if (CAT(ptr, ID) != NULL && virt_stack_commit(CAT(virt, ID))) { \
		for (usz CAT(i, ID) = 0; CAT(i, ID) < CAT(count, ID); CAT(i, ID) += 1) { \
			CAT(ptr, ID)[CAT(i, ID)] = CAT(value, ID); \
		}; \
	}; \
	\
	CAT(ptr, ID); \
})

#define virt_stack_alloc(virt, count, value...) _urm_virt_stack_alloc(UNIQ(_urm_virt_stack_alloc_), virt, count, value)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// virt dual

typedef struct {
	// allocate like all memory is commited
	DUAL_EMBED(slice, bump, stack);
	// not comitted region between arena and stack.
	struct {
		uptr pos_up;
		uptr pos;
		// if [step] == 0 then working on slices, otherwise we have pages.
		usz step;
	} page;
	// TODO: use 2 quotes: up and down - bound generic parent.
	//      parent
	//     /      \
	//    up     down
	Quota * quota;
} VirtDual;

#endif // !RK_MEM_VIRT_VIRT_H
