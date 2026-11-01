#ifndef STD_MEM_HEAP_H
#define STD_MEM_HEAP_H

#include "std/mem/pool.h"

// |================================================================================================|
// |> HEAP                                                                                          |
// |================================================================================================|

#define HEAP_BACKET_SIZE MB(64)
#define HEAP_BACKET_COUNT 12
#define HEAP_BACKET_MAX_ITEM_SIZE (1ULL << (2 + HEAP_BACKET_COUNT))
typedef struct HeapBacket HeapBacket;

// one pool item
struct HeapBacket {
	PoolGen pool;
	HeapBacket * free;
	HeapBacket * full;
	u8 _PAD[8];
};

// pools place
typedef POOL(HeapBacket) HeapBackets;
// 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
typedef struct { HeapBackets pools; } Heap;

b8 heap_init(Heap * heap);
b8 heap_deinit(Heap * heap);

Reg heap_gen_alloc(Heap * heap, u64 size, u32 align);
b8  heap_gen_free(Heap * heap, Reg reg);

RegMan heap_as_man(Heap * heap);

// #define heap_alloc(__heap__, __T__)

#endif // !STD_MEM_HEAP_H
