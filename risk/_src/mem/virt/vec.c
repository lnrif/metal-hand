#include "mem/virt/vec.h"
#include "mem/virt/page.h"

SliceVec slice_vec_init_ex(void * ptr, usz len, usz size, usz align) {
	ASSERT_DEBUG(((uptr)ptr & (align - 1)) == 0, "expected aligned [ptr]");
	return (SliceVec){.ptr = (uptr)ptr, .len = 0, .cap = len / size};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vec

VirtVec virt_vec_init_ex(Memory * mem, usz item_size, usz item_align, VirtVecOpt opt) {
	ASSERT_DEBUG(mem != NULL, "expected [mem]");
	UNUSED(item_size);

	usz const page = mem_commit_align();

	ASSERT_DEBUG(
		opt.step == 0 || (opt.step & (opt.step - 1)) == 0,
		"expected that [step] is power of 2 or set to 0 (default)"
	); usz const step = (opt.step < page) ? page : opt.step;

	ASSERT_DEBUG(
		opt.size == 0 || (opt.size & (step - 1)) == 0,
		"expected that [size] is aligned with [step] or set to 0 (default)"
	); usz const size = (opt.size < step) ? step : ((opt.size - 1 + step) & ~(step - 1));

	Memory own;
	ASSERT_DEBUG(mem_split(mem, &own, size, item_align), "");

	uptr const beg = own.beg;
	uptr const end = own.end;

	return (VirtVec){
		.ptr = beg, .len = 0, .cap = 0,
		.page = {.pos = beg, .end = end, .step = step},
		// .item = {.size = size, .align = align},
		.quota = own.quota,
	};
};

bool virt_vec_grow_one_impl(VirtVec * vec, usz size, usz align) {
	// ASSERT_DEBUG(vec->item.size == size && vec->item.align == align, "item layout mismatch");
	ASSERT(vec->len >= vec->cap, "call '%s' only when you real grow to grow!", __func__);
	UNUSED(size); UNUSED(align);

	uptr const mask = vec->page.step - 1;
	uptr const old  = vec->page.pos;

	uptr const pos  = vec->ptr + (vec->len + 1) * size;
	uptr const new  = (pos + mask) & ~mask;
	usz  const len  = new - old;

	if (!quota_acquare(vec->quota, len)) return FALSE;
	if (!mem_commit((void*)old, len))    return FALSE;

	vec->page.pos = new;
	vec->cap = (vec->page.pos - (uptr)vec->ptr) / size;

	return TRUE;
};

bool virt_vec_grow_one_ex(VirtVec * vec, usz size, usz align) {
	// ASSERT_DEBUG(vec->item.size == size && vec->item.align == align, "item layout mismatch");
	return vec->len < vec->cap || virt_vec_grow_one_impl(vec, size, align);
};

