#include "mem/virt/pool.h"

MemPool mem_pool_init_ex(
	CallLoc call,
	Memory * mem,
	usz pool_size,
	usz slot_size
) {
	ASSERT_DEBUG_AT(call,
		pool_size != 0 && (pool_size & (pool_size - 1)) == 0,
		"expected that [pool_size] is shift of 2"
	);

	ASSERT_DEBUG_AT(call,
		slot_size != 0 && (slot_size & (slot_size - 1)) == 0,
		"expected that [slot_size] is shift of 2"
	);

	ASSERT_DEBUG_AT(call,
		4 <= slot_size && slot_size <= KB(16),
		"expected that [slot_size] not greater then 16KB and at least 4B"
	);

	usz const page = mem_commit_align();

	ASSERT_DEBUG_AT(call,
		page <= pool_size && pool_size <= MB(256),
		"%lluKB <= [pool_size] <= %lluKB",
		page >> 10,
		MB(256) >> 10
	);

	// 2^x / 2^y = 2^(x - y) (no remainder)
	// ASSERT_DEBUG_AT(call,
	// 	(pool_size & (slot_size - 1)) == 0,
	// 	"expected that [pool_size] aligned by [slot_size]"
	// );

	ASSERT_DEBUG_AT(call,
		pool_size / slot_size <= U32_MAX,
		// index starts from 0, so +1
		// but we store [free_count: u32], so not
		"slot count is limited to %hu, you use %lluKB / %lluB = %llu ",
		U32_MAX,
		pool_size >> 10,
		slot_size,
		pool_size / slot_size
	);

	Memory own;
	ASSERT_DEBUG_AT(call, mem_split(mem, &own, pool_size, slot_size), "");

	u32 const slot_count       = (u32)(pool_size / slot_size);
	u8  const slot_count_shift = (u8)CTZ(slot_count);

	return (MemPool){
		.beg = own.beg, .end = own.end,
		.commit_index = 0, .free_index = 0, .free_count = slot_count,
		.slot_count_shift = slot_count_shift,
		.slot_size_shift = (u8)CTZ(slot_size),
	};
};

uptr mem_pool_alloc_ptr(MemPool * pool) {
	ASSERT_DEBUG(pool != NULL, "expected [pool]");

	if (pool->free_count == 0) goto failed;

	uptr const pool_begin = pool->beg;
	uptr const free       = pool_begin + ((uptr)pool->free_index << pool->slot_size_shift);

	if (pool->free_index >= pool->commit_index) {
		// only last commited slot pointing at next slot that not commited,
		// so it pointer is page start (no align needed)
		ASSERT_DEBUG(pool->free_index == pool->commit_index, "");

		usz const slot_size = 1 << pool->slot_size_shift;
		usz const page = mem_commit_align();
		usz const len  = mem_align_ceil(slot_size, page);
		if (!mem_commit((void*)free, len)) goto failed;

		uptr i = free;
		for (;;) {
			uptr const next = i + slot_size;
			*(MemPoolIndex*)i = (MemPoolIndex)((next - pool_begin) >> pool->slot_size_shift);
			i = next;
			if (i >= free + len) break;
		};

		pool->commit_index += len >> pool->slot_size_shift;
	};

	pool->free_index = *(MemPoolIndex*)free;
	pool->free_count -= 1;

	#ifdef RK_MEM_POOLS_PRINT
		printf(
			"%s() -> { ptr: %p, len: %llu }\n",
			__func__,
			(void*)free,
			1ULL << (usz)pool->slot_size_shift
		);
	#endif

	return free;

failed:
	#ifdef RK_MEM_POOLS_PRINT
		printf("%s() -> { ptr: NULL, len: 0 }\n", __func__);
	#endif

	return 0;
};

MemPoolIndex mem_pool_alloc_idx(MemPool * pool) {
	uptr const ptr = mem_pool_alloc_ptr(pool);
	if (ptr == 0) return MEM_POOL_INDEX_INVALID;
	return (MemPoolIndex)((ptr - pool->beg) >> pool->slot_size_shift);
};

bool mem_pool_free_ptr(MemPool * pool, uptr ptr) {
	ASSERT_DEBUG(pool != NULL, "expected [pool]");

	if (ptr == 0) goto ok;
	if (ptr < pool->beg || pool->end <= ptr) goto err;

	usz const slot_size = 1 << pool->slot_size_shift;
	if ((ptr & (slot_size - 1)) != 0) goto err;

	*(MemPoolIndex*)ptr = pool->free_index;
	pool->free_index = (MemPoolIndex)((ptr - pool->beg) >> pool->slot_size_shift);
	pool->free_count += 1;

	if (pool->free_count == 1 << pool->slot_count_shift) {
		usz const len = 1 << (pool->slot_count_shift + pool->slot_size_shift);
		ASSERT_DEBUG(mem_decommit((void*)pool->beg, len), "failed to decommit");
		pool->commit_index = 0;
		pool->free_index   = 0;
	};

	ok:
	#ifdef RK_MEM_POOLS_PRINT
		printf(
			"%s({ ptr: %p, len: %llu }) -> TRUE\n",
			__func__, (void*)ptr, 1ULL << (usz)pool->slot_size_shift
		);
	#endif
	return TRUE;

err:
	#ifdef RK_MEM_POOLS_PRINT
		printf(
			"%s({ ptr: %p, len: %llu }) -> FALSE\n",
			__func__, (void*)ptr, 1ULL << (usz)pool->slot_size_shift
		);
	#endif
	return FALSE;
};

bool mem_pool_free_idx(MemPool * pool, MemPoolIndex idx) {
	ASSERT_DEBUG(pool != NULL, "expected [pool]");
	if (idx == MEM_POOL_INDEX_INVALID) return TRUE;
	if (idx > pool->commit_index) return FALSE;
	return mem_pool_free_ptr(pool, ((uptr)idx << pool->slot_size_shift) + pool->beg);
};

uptr mem_pool_get(MemPool * pool, MemPoolIndex idx) {
	ASSERT_DEBUG(pool != NULL, "expected [pool]");
	if (idx == MEM_POOL_INDEX_INVALID) return 0;
	if (idx > pool->commit_index) return 0;
	return (idx << pool->slot_size_shift) + pool->beg;
};

