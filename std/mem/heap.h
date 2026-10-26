#ifndef STD_MEM_HEAP_H
#define STD_MEM_HEAP_H

#include "std/mem/list.h"
#include "std/mem/page.h"
#include "std/mem/pool.h"

// |================================================================================================|
// |> HEAP                                                                                          |
// |================================================================================================|

#define HEAP_POOL_SIZE MB(4)
typedef struct HeapPool HeapPool;

// one pool item
struct HeapPool { PoolGen pool; HeapPool * free; HeapPool * full; };
// entire linked list of pools
typedef struct { HeapPool * free; HeapPool * full; } HeapKind;
// 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
typedef struct { PoolGen pools; } Heap;

Heap heap_init();
b8   heap_deinit(Heap * heap);

Reg heap_gen_alloc(Heap * heap, u64 size, u32 align);
b8  heap_gen_free (Heap * heap, Reg reg);

typedef LIST(Pages) MemPersistent;

typedef struct {
	Heap heap;
	MemPersistent persistent;
} MemGen;

#endif // !STD_MEM_HEAP_H
