#include "mem/virt/virt.h"
#include "mem/real/bump.h"
#include "mem/virt/page.h"
#include "mem/quota.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// virt up

VirtBump virt_bump_init_ex(Memory * mem, VirtInitOpts opt) {
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
	ASSERT(mem_split(mem, &own, size, step), "failed");
	Bump bump = bump_init(own.beg, own.end);

	return (VirtBump){
		.bump = bump,
		.page = {.pos = bump.beg, .step = step},
		.quota = own.quota,
	};
};

void virt_bump_shrink(VirtBump * virt) {
	// 1) found bounds
	uptr const beg = mem_align_ceil(virt->bump.pos, mem_commit_align());
	uptr const top = virt->page.pos;
	if (beg >= top) return;

	// 2) decommit memory
	usz const len = top - beg;
	ASSERT_DEBUG(mem_decommit((void*)beg, len), "decommit failed");
	virt->page.pos = beg;

	// 3) release quota
	quota_release(virt->quota, len);
};

void virt_bump_decommit(VirtBump * virt) {
	// 1) reset [virt]
	virt_bump_reset(virt);

	// 2) decommit pages
	uptr const beg = virt->bump.beg;
	uptr const top = virt->page.pos;
	usz const len = top - beg;
	ASSERT_DEBUG(mem_decommit((void*)beg, len), "decommit failed");
	virt->page.pos = virt->bump.beg;

	// 3) release quota
	quota_release(virt->quota, len);
};

bool virt_bump_commit_impl(VirtBump * virt) {
	ASSERT_DEBUG(virt != NULL, "expected [virt]");
	ASSERT_DEBUG(virt->page.pos < virt->bump.pos, "call `%s` only when you need to grow!", __func__);

	usz  const mask = virt->page.step - 1;
	uptr const old  = virt->page.pos;
	uptr const new  = (virt->bump.pos + mask) & ~mask;
	usz  const len  = new - old;

	if (!quota_acquare(virt->quota, len)) return FALSE;
	if (!mem_commit((void*)old, len))     return FALSE;

	virt->page.pos = new;
	return TRUE;
};

bool virt_bump_commit(VirtBump * virt) {
	return virt->page.pos >= virt->bump.pos || virt_bump_commit_impl(virt);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// virt down

VirtStack virt_stack_init_ex(Memory * mem, VirtInitOpts opt) {
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
	ASSERT_DEBUG(mem_split(mem, &own, size, step), "");

	return (VirtStack){
		.stack = own.stack,
		.page = {.pos = own.stack.end, .step = step},
		.quota = own.quota,
	};
};

void virt_stack_shrink(VirtStack * virt) {
	// 1) found bounds
	usz const align = mem_commit_align();
	uptr const beg = virt->page.pos;
	uptr const top = virt->stack.pos & ~(align - 1);
	if (beg >= top) return;

	// 2) decommit memory
	usz const len = top - beg;
	ASSERT_DEBUG(mem_decommit((void*)beg, len), "decommit failed");
	virt->page.pos = virt->stack.end;

	// 3) release quota
	quota_release(virt->quota, len);
};

void virt_stack_decommit(VirtStack * virt) {
	// 1) reset [virt]
	virt_stack_reset(virt);

	// 2) decommit pages
	uptr const beg = virt->page.pos;
	uptr const top = virt->stack.end;
	usz  const len = top - beg;
	ASSERT_DEBUG(mem_decommit((void*)beg, len), "decommit failed");
	virt->page.pos = virt->stack.end;

	// 3) release quota
	quota_release(virt->quota, len);
};

bool virt_stack_commit_impl(VirtStack * virt) {
	ASSERT_DEBUG(virt->page.pos > virt->stack.pos, "call `%s` only when you need to grow!", __func__);

	usz  const mask = virt->page.step - 1;
	uptr const old  = virt->page.pos;
	uptr const new  = virt->stack.pos & ~mask;

	usz const len = old - new;
	if (!quota_acquare(virt->quota, len)) return FALSE;
	if (!mem_commit((void*)new, len))     return FALSE;

	virt->page.pos = new;
	return TRUE;
};

bool virt_stack_commit(VirtStack * virt) {
	return virt->page.pos <= virt->stack.pos || virt_stack_commit_impl(virt);
};

uptr virt_stack_mark(VirtStack const * virt) {
	ASSERT_DEBUG(virt != NULL, "expected [virt]");
	return stack_mark(&virt->stack);
};

void virt_stack_load(VirtStack * virt, uptr mark) {
	ASSERT_DEBUG(virt != NULL, "expected [virt]");
	stack_load(&virt->stack, mark);
};

