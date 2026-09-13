#ifndef RK_MEM_REAL_STACK_H
#define RK_MEM_REAL_STACK_H

#include "core/types.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// stack

///   |---free---|---use---|
///   |??????????xxxxxxxxxx|
///   ^ beg <--- ^ pos     ^ end
typedef struct {
	uptr beg;
	uptr pos;
	uptr end;
} Stack;

#define STACK_EMBED(stack, beg, pos, end) \
	union { \
		Stack stack; \
		struct { uptr beg; uptr pos; uptr end; }; \
	}

////////////////////////////////
// constructors

/// Initialize stack down (end to beg) from bounds (or panic in debug mode).
Stack stack_init(uptr beg, uptr end);

/// Initialize stack down (end to beg) from slice (or panic in debug mode).
Stack stack_from_slice(void * ptr, usz len);

/// Initialize stack down (end to beg) from array.
#define stack_from_array(arr) \
	((Stack){ \
		.beg = (uptr)(arr), \
		.pos = (uptr)(arr) + sizeof(arr), \
		.end = (uptr)(arr) + sizeof(arr), \
	}) // (uptr)(arr) + sizeof(arr) cannot overflow

////////////////////////////////
// temporary

typedef struct { Stack * get; uptr mark; } StackTemp;

StackTemp stack_temp(Stack * stack);
void      stack_finish(StackTemp temp);

////////////////////////////////
// mark & reset

uptr stack_mark(Stack const * stack);
void stack_load(Stack * stack, uptr mark);
void stack_reset(Stack * stack);

////////////////////////////////
// split

bool stack_split(Stack * src, Stack * dst, usz size, usz align);

////////////////////////////////
// raw alloc

/// Allocate raw block:
/// - if [stack]    is NULL       then returns NULL.
/// - if [align] is not power of 2 then panic.
/// You can allocate [size] == 0.
uptr stack_raw(Stack * stack, usz count, usz size, usz align);

////////////////////////////////
// raw alloc wrappers

#define stack_thing(stack, T) ((T*)stack_raw(stack, 1,              sizeof(T), alignof(T)))
#define stack_block(stack, T) ((T*)stack_raw(stack, 1, offsetof(T, BLOCK_END), alignof(T)))

#define stack_array(stack, T, count) ({ \
	STATIC_ASSERT( \
		(sizeof(T) & (alignof(T) - 1)) == 0 || sizeof(T) <= alignof(T), \
		"to allocate array of item with [size] that [size] must be aligned with [align]" \
	);\
	(T*)stack_raw(stack, count, sizeof(T), alignof(T)); \
})

////////////////////////////////
// alloc & init

#define _stack_alloc(ID, _stack, _count, _value...) ({ \
	Cursor *   CAT(stack, ID) = (_stack);   \
	usz            CAT(count,  ID) = (_count); \
	typeof(_value) CAT(value,  ID) = (_value); \
	\
	typeof(_value) * CAT(ptr, ID) = \
		stack_array(CAT(stack, ID), typeof(_value), CAT(count, ID)); \
	\
	if (CAT(ptr, ID) != NULL) { \
		for (usz CAT(i, ID) = 0; CAT(i, ID) < CAT(count, ID); CAT(i, ID) += 1) { \
			CAT(ptr, ID)[CAT(i, ID)] = CAT(value, ID); \
		}; \
	}; \
	\
	CAT(ptr, ID); \
})

#define stack_alloc(stack, count, value...) _stack_alloc(UNIQ(_stack_alloc_), stack, count, value)

#endif // !RK_MEM_REAL_STACK_H
