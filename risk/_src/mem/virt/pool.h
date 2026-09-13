#ifndef RK_MEM_POOL_H
#define RK_MEM_POOL_H

#include "mem/virt/page.h"

typedef struct MemPool MemPool;
typedef u32            MemPoolIndex;

#define MEM_POOL_INDEX_INVALID U32_MAX

struct MemPool {
	uptr beg; uptr end;
	MemPoolIndex commit_index, free_index, free_count;
	u8 slot_count_shift; // slot_count = 1 << slot_count_shift;
	u8 slot_size_shift;  // slot_size  = 1 << slot_size_shift;
};

MemPool mem_pool_init(Memory * mem, usz slot_count, usz slot_size);

uptr         mem_pool_alloc_ptr(MemPool * pool);
MemPoolIndex mem_pool_alloc_idx(MemPool * pool);

bool mem_pool_free_ptr(MemPool * pool, uptr ptr);
bool mem_pool_free_idx(MemPool * pool, MemPoolIndex idx);

uptr mem_pool_get(MemPool * pool, MemPoolIndex idx);

#endif // !RK_MEM_POOL_H
