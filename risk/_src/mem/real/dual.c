#include "mem/real/dual.h"
#include "trc/assert.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dual

////////////////////////////////
// constructors

Dual dual_init(void * ptr, usz len) {
	ASSERT_DEBUG(ptr != NULL, "expected non-null [ptr]");
	uptr const beg = (uptr)ptr; uptr end;
	ASSERT_DEBUG(ADD_OVER_DBG(beg, len, &end), "overflow");
	return (Dual){.beg = beg, .bump_pos = beg, .stack_pos = end, .end = end};
};

////////////////////////////////
// temporary

DualTemp dual_temp(Dual * slice) {
	return (DualTemp){.get = slice, .bump_mark = slice->bump_pos, .stack_mark = slice->stack_pos};
};

void dual_finish(DualTemp temp) {
	temp.get->bump_pos  = temp.bump_mark;
	temp.get->stack_pos = temp.stack_mark;
};

////////////////////////////////
// mark & reset

void dual_reset(Dual * dual) {
	ASSERT_DEBUG(dual != NULL, "expected [dual]");
	dual->bump_pos  = dual->beg;
	dual->stack_pos = dual->end;
};

