#include "std/mem/pool.h"

#include "std/core.h"
#include "std/mem/core.h"
#include "std/mem/pool.h"

// |================================================================================================|
// |> POOL REG                                                                                      |
// |================================================================================================|

PoolItem pool_item_init(u32 size) {
	if (size == 0 || (size & (size - 1)) != 0) return (PoolItem){.is_valid = false};
	size = MAX((u32)sizeof(uptr), size);
	return (PoolItem){.size = size, .shift = (u32)CTZ(size)};
};

b8 pool_gen_init(PoolGen * gen, PoolId count, PoolItem item) {
	Pages pages = pages_reserve(item.size * count);
	if (!pages.is_valid) return false;
	*gen = (PoolGen){.ps = pages, .free = 0, .null = 0};
	return true;
};

void pool_gen_deinit(PoolGen * gen) {
	pages_release(&gen->ps); gen->free = 0; gen->null = 0;
};

b8 pool_gen_chain(PoolGen * gen, uptr beg, uptr end, PoolItem item) {
	if (beg > end || end - beg == 0) return false;
	if ((beg & (item.size - 1)) != 0) return false;
	if ((end & (item.size - 1)) != 0) return false;
	if (beg < gen->beg || gen->pos < end) return false;

	if (gen->free == 0) gen->free = beg;
	uptr const new_null = end - item.size;
	if (gen->null != 0) *gen->null_ptr = new_null;
	gen->null = new_null; *gen->null_ptr = 0;

	uptr ptr = beg;
	while (ptr < gen->null) {
		*(uptr *)ptr = ptr + item.size;
		ptr += item.size;
	};

	return true;
};

b8 pool_gen_grow(PoolGen * gen, PoolId count, PoolItem item) {
	if (count == 0) return true;

	uptr const pos = mem_align_up(
		gen->pos + (count << item.shift),
		pages_align_atom()
	);

	if (pos == U64_MAX || gen->end < pos) return false;
	if (!pages_read_write(REG(gen->pos, pos - gen->pos))) return false;

	uptr const beg = gen->pos; gen->pos = pos;
	return pool_gen_chain(gen, beg, pos, item);
};

void * pool_gen_alloc(PoolGen * gen, PoolItem item) {
	if (gen->free == 0 && !pool_gen_grow(gen, 1, item)) return 0;
	uptr free = gen->free;
	if (free != 0) gen->free = *(uptr *)free;
	return (void*)free;
};

b8 pool_gen_free(PoolGen * gen, void * ptr, PoolItem item) {
	b8 const in      = gen->beg <= (uptr)ptr && (uptr)ptr < gen->pos;
	b8 const aligned = ((uptr)ptr & (item.size - 1)) == 0;
	if (!in || !aligned) return false;

	*(uptr *)ptr = gen->free;
	gen->free = (uptr)ptr;

	return true;
};

void pool_gen_reset(PoolGen * gen, PoolItem item) {
	pool_gen_chain(gen, gen->beg, gen->pos, item);
};

void * pool_gen_mut(PoolGen * gen, PoolId id, PoolItem item) {
	return (void *)pool_gen_ref(gen, id, item);
};

void const * pool_gen_ref(PoolGen const * gen, PoolId id, PoolItem item) {
	if (id == POOL_ID_NIL) return 0;
	uptr ptr = gen->beg + item.size * (id - 1);
	if (ptr >= gen->pos) return 0;
	return (void const *)ptr;
};

PoolId pool_gen_id(PoolGen const * gen, void const * ptr, PoolItem item) {
   if ((uptr)ptr < gen->beg || gen->pos <= (uptr)ptr) return POOL_ID_NIL;
	return (((uptr)ptr - gen->beg) >> item.shift) + 1;
};

