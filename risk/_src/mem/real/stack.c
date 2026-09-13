#include "mem/real/stack.h"
#include "trc/assert.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// slice down

////////////////////////////////
// constructors

Stack stack_init(uptr beg, uptr end) {
	ASSERT_DEBUG(beg <= end, "invalid bounds, expected that [beg] <= [end]");
	ASSERT_DEBUG(beg != 0 || (beg == 0 && end == 0), "invalid [slice]");
	return (Stack){.beg = beg, .pos = end, .end = end};
};

Stack stack_from_slice(void * ptr, usz len) {
	ASSERT_DEBUG(ptr != NULL || (ptr == NULL && len == 0), "invalid [slice]");
	uptr beg = (uptr)ptr; uptr end = 0;
	ASSERT_DEBUG(!ADD_OVER_DBG(beg, len, &end), "overflow");
	return stack_init(beg, end);
};

////////////////////////////////
// temporary

StackTemp stack_temp(Stack * slice) {
	return (StackTemp){.get = slice, .mark = stack_mark(slice)};
};

void stack_finish(StackTemp temp) {
	stack_load(temp.get, temp.mark);
};

////////////////////////////////
// mark & reset

uptr stack_mark(Stack const * slice) {
	ASSERT_DEBUG(slice != NULL, "expected [slice]");
	return slice->pos;
};

void stack_load(Stack * slice, uptr mark) {
	ASSERT_DEBUG(slice != NULL, "expected [slice]");
	ASSERT_DEBUG(slice->beg <= mark && mark <= slice->end, "[mark] is out of [slice]");
	if (slice->pos <= mark) slice->pos = mark;
};

void stack_reset(Stack * slice) {
	ASSERT_DEBUG(slice != NULL, "expected [slice]");
	slice->pos = slice->end;
};

////////////////////////////////
// split

bool stack_split(Stack * src, Stack * dst, usz size, usz align) {
	ASSERT_DEBUG(src != NULL, "expected [src]");
	ASSERT_DEBUG(dst != NULL, "expected [dst]");

	uptr mem = stack_raw(src, 1, size, align);
	if (mem == 0) return FALSE;

	*dst = stack_init(mem, mem + size);
	return TRUE;
};

////////////////////////////////
// raw alloc

uptr stack_raw(Stack * slice, usz count, usz size, usz align) {
	ASSERT_DEBUG(slice != NULL, "invalid [slice]");
	ASSERT_DEBUG(ALIGN_IS_SANE(align), "invalid [align]");
	ASSERT_DEBUG(
		slice->beg <= slice->pos && slice->pos <= slice->end,
		"invalid invariant"
	);
	ASSERT_DEBUG(
		count <= 1 || size <= align || (size & (align - 1)) == 0,
		"to allocate array of item with [size] that [size] must be aligned with [align]"
	);

	// ================ evaluation step ================
	// 1.1) evaluate total size
	uptr total; if (MUL_OVER_DBG(count, size, &total)) return 0;

	// 1.2) evaluate position
	uptr pos; if (SUB_OVER_DBG(slice->pos, total, &pos)) return 0;
	pos &= ~(align - 1);

	// ================== check step ===================
	// 1.1) check bounds
	if (pos < slice->beg) return 0;
	// 1.2) allocation success
	slice->pos = pos;

	return pos;
};

