#include "mem/virt/pools.h"

MemPools mem_pools_init_ex(
	CallLoc call,
	Memory * mem,
	usz chunk_count,
	usz chunk_size,
	usz slot_size
) {
	ASSERT_DEBUG_AT(call,
		chunk_size != 0 && (chunk_size & (chunk_size - 1)) == 0,
		"expected that [chunk_size] is shift of 2"
	);

	ASSERT_DEBUG_AT(call,
		slot_size != 0 && (slot_size & (slot_size - 1)) == 0,
		"expected that [slot_size] is shift of 2"
	);

	ASSERT_DEBUG_AT(call,
		 2 <= slot_size && slot_size <= KB(16),
		"expected that [slot_size] not greater then 16KB and at least 2B"
	);

	usz const page = mem_commit_align();

	ASSERT_DEBUG_AT(call,
		page <= chunk_size && chunk_size <= MB(256),
		"%lluKB <= [chunk_size] <= %lluKB",
		page >> 10,
		MB(256) >> 10
	);

	// 2^x / 2^y = 2^(x - y) (no remainder)
	// ASSERT_DEBUG_AT(call,
	// 	(chunk_size & (slot_size - 1)) == 0,
	// 	"expected that [chunk_size] aligned by [slot_size]"
	// );

	ASSERT_DEBUG_AT(call,
		chunk_size / slot_size <= U16_MAX,
		// index starts from 0, so +1
		// but we store [free_count: u16], so not
		"slot count is limited to %hu, you use %lluKB / %lluB = %llu ",
		U16_MAX,
		chunk_size >> 10,
		slot_size,
		chunk_size / slot_size
	);

	ASSERT_DEBUG_AT(call, chunk_count <= 16, "[chunk_count] limited to 16");

	Memory own;
	ASSERT_DEBUG_AT(call,mem_split(mem, &own, chunk_count * chunk_size, slot_size), "");

	u16 const slot_count = (u16)(chunk_size / slot_size);
	u8 const slot_count_shift = (u8)CTZ(slot_count);

	MemPools pools = (MemPools){
		// .chunks = {0},
		// .lookup = {0},
		.beg = own.beg, .end = own.end,
		.slot_count_shift = slot_count_shift,
		.slot_size_shift = (u8)CTZ(slot_size),
	};

	usz i = 0; usz free = pools.end - pools.beg;
	while (free != 0) {
		ASSERT_DEBUG_AT(call,
			!SUB_OVER(free, chunk_size, &free),
			"expected that [free] %% [chunk_size] == 0"
		);

		ASSERT_DEBUG_AT(call,
			i < LEN(pools.chunks),
			"expected that loop is end when [free] == 0, (i: %llu)",
			i
		);

		pools.chunks[i] = (MemChunk){
			.commit_index = 0,
			.free_index   = 0,
			.free_count   = slot_count,
		};

		i += 1;
	};

	while (i < LEN(pools.chunks)) {
		pools.chunks[i] = (MemChunk){
			.commit_index = 0,
			.free_index   = 0,
			.free_count   = 0,
		};
		i += 1;
	};

	// [0..= 4] = 0                                0 << (0 << 2) == 0 << 0
	// [5..= 8] = 1                                1 << (1 << 2) == 1 << 4
	// [9..=12] = 2                                2 << (2 << 2) == 2 << 8
	// ...
	for (u64 j = 0; j < 16; j += 1) pools.lookup |= j << (j << 2);

	return pools;
};

// #define RK_MEM_POOLS_PRINT

uptr mem_pools_alloc(MemPools * pools) {
	ASSERT_DEBUG(pools != NULL, "expected [pools]");

	usz        chunk_index = pools->lookup & 0xF;
	MemChunk * chunk       = &pools->chunks[chunk_index];

	if (chunk->free_count == 0) goto failed;

	usz const  chunk_shift = pools->slot_count_shift + pools->slot_size_shift;
	uptr const chunk_begin = pools->beg + (chunk_index << chunk_shift);
	uptr const free        = chunk_begin + ((uptr)chunk->free_index << pools->slot_size_shift);

	if (chunk->free_index >= chunk->commit_index) {
		// only last commited slot pointing at next slot that not commited,
		// so it pointer is page start (no align needed)
		ASSERT_DEBUG(chunk->free_index == chunk->commit_index, "");

		usz const slot_size = 1 << pools->slot_size_shift;
		usz const page = mem_commit_align();
		usz const len  = mem_align_ceil(slot_size, page);
		if (!mem_commit((void*)free, len)) goto failed;

		usz const added = len >> pools->slot_size_shift;
		chunk->commit_index += added;

		uptr i = free;
		for (;;) {
			uptr const next = i + slot_size;
			*(u16*)i = (u16)((next - chunk_begin) >> pools->slot_size_shift);
			i = next;
			if (i >= free + len) break;
		};
	};

	chunk->free_index = *(u16*)free;
	chunk->free_count -= 1;

	if (chunk->free_count == 0) {
		pools->lookup = ((pools->lookup & 0xF) << 60) | (pools->lookup >> 4);
	};

	#ifdef RK_MEM_POOLS_PRINT
		printf(
			"%s() -> { ptr: %p, len: %llu }\n",
			__func__,
			(void*)free,
			1ULL << (usz)pools->slot_size_shift
		);
	#endif

	return free;

failed:
	#ifdef RK_MEM_POOLS_PRINT
		printf("%s() -> { ptr: NULL, len: 0 }\n", __func__);
	#endif

	return 0;
};

bool mem_pools_free(MemPools * pools, uptr ptr) {
	ASSERT_DEBUG(pools != NULL, "expected [pools]");

	if (ptr == 0)                              goto ok;
	if (ptr < pools->beg || pools->end <= ptr) goto err;

	usz const slot_size = 1 << pools->slot_size_shift;
	if ((ptr & (slot_size - 1)) != 0)          goto err;

	usz  const chunk_shift = pools->slot_count_shift + pools->slot_size_shift;
	usz  const chunk_index = (ptr - pools->beg) >> chunk_shift;
	uptr const chunk_begin = pools->beg + (chunk_index << chunk_shift);

	MemChunk * const chunk = &pools->chunks[chunk_index];
	*(u16*)ptr = chunk->free_index;
	chunk->free_index = (u16)((ptr - chunk_begin) >> pools->slot_size_shift);
	chunk->free_count += 1;

	if (chunk->free_count == 1 << pools->slot_count_shift) {
		ASSERT_DEBUG(mem_decommit((void*)chunk_begin, 1 << chunk_shift), "failed to decommit");
		chunk->commit_index = 0;
		chunk->free_index   = 0;
	};

	u64 lookup_bit = USZ_MAX;
	for (usz i = 0; i < 64; i += 4) {
		if (chunk_index != ((pools->lookup >> i) & 0xF)) continue;
		lookup_bit = i;
		break;
	};

	usz i = lookup_bit;
	for (;;) {
		if (i == 60) break;

		u64 const ai = (pools->lookup >> (i + 0)) & 0xF;
		MemChunk * a = &pools->chunks[ai];

		u64 const bi = (pools->lookup >> (i + 4)) & 0xF;
		MemChunk * b = &pools->chunks[bi];

		if (b->free_count == 0 || a->free_count <= b->free_count) break;

		u64 const blank = pools->lookup & ~(0xFFULL << i);
		pools->lookup = blank | (ai << (i + 4)) | (bi << (i + 0));
		i += 4;
	};

ok:
	#ifdef RK_MEM_POOLS_PRINT
		printf(
			"%s({ ptr: %p, len: %llu }) -> TRUE\n",
			__func__, (void*)ptr, 1ULL << (usz)pools->slot_size_shift
		);
	#endif
	return TRUE;

err:
	#ifdef RK_MEM_POOLS_PRINT
		printf(
			"%s({ ptr: %p, len: %llu }) -> FALSE\n",
			__func__, (void*)ptr, 1ULL << (usz)pools->slot_size_shift
		);
	#endif
	return FALSE;
};

