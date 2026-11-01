#include "std/mem/heap.h"
#include "std/core.h"
#include "std/mem/page.h"
#include "std/mem/pool.h"
#include "std/mem/reg.h"
#include "std/mem/core.h"

b8 heap_init(Heap * heap) {
	HeapBackets pools; // 4096 * 4MB = 16 GB memory
	if (!pool_init(&pools, 1 << 12)) return false;

	// 8, 16, 32, 64, 128, 256,
	// 512, 1024, 2048, 4096, 8192, 16384
	for (u64 i = 0; i < HEAP_BACKET_COUNT; i += 1) {
		HeapBacket * const backet = pool_alloc(&pools);
		if (backet == 0) return false;

		u32 const backet_size = 8u << i;
		u32 const backet_count = HEAP_BACKET_SIZE / backet_size;
		PoolItem const backet_item = pool_item_init(backet_size);
		if (!pool_gen_init(&backet->pool, backet_count, backet_item)) return false;

		backet->free = 0; backet->full = 0;
	};

	heap->pools = pools;
	return true;
};

b8 heap_deinit(Heap * heap) {
	// TODO: remove all buckets
	for (u64 i = 0; i < HEAP_BACKET_COUNT; i += 1) {
		HeapBacket * backet = pool_mut(&heap->pools, i + 1);
		if (backet == 0) return false;
		pool_gen_deinit(&backet->pool);
	};
	return true;
};

u64 mem_pow2_up(u64 x) {
	if (x <= 1) return 1;
	if (x > (1ULL << 63)) return 0;
	return 1ULL << (64 - CLZ(x - 1));
};

Reg heap_gen_alloc(Heap * heap, u64 size, u32 align) {
	u64 const total = mem_pow2_up(MAX(size, align));
	if (total == 0) return REG_NIL;

	if (total <= HEAP_BACKET_MAX_ITEM_SIZE) {
		u64 const pow = MAX(3ULL, CTZ(total));
		u64 const index = pow - 2;
		HeapBacket * backet = pool_mut(&heap->pools, index);
		// TODO: add search and buckets alloc
		void * ptr = pool_gen_alloc(&backet->pool, pool_item_init((u32)total));
		if (ptr != 0) return REG((uptr)ptr, 1 << pow);
	};

	Pages pages = pages_reserve(total);
	if (!pages.is_valid) return REG_NIL;

	Pages mem = pages_chop(&pages, total, 1);
	Reg reg = REG(mem.beg, mem.end - mem.beg);
	if (!mem.is_valid || !pages_read_write(reg)) return REG_NIL;

	return reg;
};

b8 heap_gen_free(Heap * heap, Reg reg) {
	if (reg.ptr == 0 || reg.len == 0) return true;

	u64 const total = mem_pow2_up(reg.len);
	if (total == 0) return false;

	if (total <= HEAP_BACKET_MAX_ITEM_SIZE) {
		u64 const pow = MAX(3ULL, CTZ(total));
		u64 const index = pow - 2;
		HeapBacket * backet = pool_mut(&heap->pools, index);
		// TODO: add search and buckets alloc
		if (pool_gen_free(&backet->pool, reg.any, pool_item_init((u32)total))) return true;
	};

	uptr const len = mem_align_up(reg.len, pages_align_atom());
	Pages ps = (Pages){.beg = reg.ptr, .end = reg.ptr + len};
	return pages_release(&ps);
};

// |================================================================================================|
// |> HEAP AS MAN                                                                                   |

Reg heap_alloc_api(void * ctx, Reg reg, RegReq req) {
	Heap * heap = ctx;

	if (req.len == 0) {
		if (reg.is_valid) heap_gen_free(heap, reg);
		return REG_NIL;
	};

	// TODO:
	if (req.dir != REG_DIR_UP) return REG_NIL;

	Reg new = heap_gen_alloc(heap, req.len, req.align);
	if (!new.is_valid) return REG_NIL;

	memcpy(new.any, reg.any, reg.len);
	heap_gen_free(heap, reg);

	return new;
};

RegMan heap_as_man(Heap * heap) { return REG_MAN(heap, heap_alloc_api); };
