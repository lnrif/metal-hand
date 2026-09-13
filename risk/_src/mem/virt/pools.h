#ifndef RK_MEM_POOLS_H
#define RK_MEM_POOLS_H

#include "trc/assert.h"
#include "mem/virt/page.h"
#include <stddef.h>

typedef struct MemPools  MemPools;
typedef struct MemChunk MemChunk;

struct MemChunk {
	u16 commit_index;
	u16 free_index;
	u16 free_count;
};

// sizeof(MemPools) == 128
struct MemPools {
	MemChunk chunks[16];
	u64 lookup;

	// memory region
	uptr beg;
	uptr end;

	// chunk meta
	u8 slot_count_shift; // slot_count = 1 << slot_count_shift;
	u8 slot_size_shift;  // slot_size  = 1 << slot_size_shift;

	u8 _pad[6];
} ALIGNED(64);

STATIC_ASSERT(sizeof(MemPools) == 128);

#define mem_pools_init(mem, chunk_count, chunk_size, slot_size) \
	mem_pools_init_ex(CALL, mem, chunk_count, chunk_size, slot_size)

MemPools mem_pools_init_ex(
	CallLoc loc,
	Memory * mem,
	usz chunk_count,
	usz chunk_size,
	usz slot_size
);

uptr    mem_pools_alloc(MemPools * pools);
bool    mem_pools_free(MemPools * pools, uptr ptr);

// // sizeof(MemPoolsMap) == 1024 == 8 * 128
// typedef struct {
// 	// [0] =   64 = 2^ 6
// 	// [1] =  128 = 2^ 7
// 	// [2] =  256 = 2^ 8
// 	// [3] =  512 = 2^ 9
// 	// [4] = 1024 = 2^10
// 	// [5] = 2048 = 2^11
// 	// [6] = 4096 = 2^12
// 	// [7] = 8192 = 2^13
// 	MemChunks chunks[8];
// } MemPools;
//
// bool mem_pools_init(MemPools * pools);
//
//
// bool mem_pools_alloc(MemPools * pools, MemAlloc * alloc, usz len);
// bool mem_pools_free(MemPools * pools, MemAlloc alloc);
//
#endif // !RK_MEM_POOLS_H
