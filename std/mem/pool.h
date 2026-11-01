#ifndef STD_MEM_POOL_H
#define STD_MEM_POOL_H

#include "std/mem/page.h"

// |================================================================================================|
// |> POOL REG                                                                                      |
// |================================================================================================|

typedef struct {
	PAGES_EMBED(ps, is_valid, beg, cur, pos, end);
	union { uptr free; uptr * free_ptr; };
	union { uptr null; uptr * null_ptr; };
} PoolGen;

STATIC_ASSERT(sizeof(PoolGen) == 40);

// item is [---size---]
// shift := @ctz(size);
typedef struct {
	union { u32 size; b32 is_valid; };
	u32 shift;
} PoolItem;

PoolItem pool_item_init(u32 size);

typedef u64 PoolId;
#define POOL_ID_NIL ((PoolId)0)

b8   pool_gen_init(PoolGen * gen, PoolId count, PoolItem item);
void pool_gen_deinit(PoolGen * gen);

b8     pool_gen_grow(PoolGen * gen, PoolId count, PoolItem item);
void * pool_gen_alloc(PoolGen * gen, PoolItem item);
b8     pool_gen_free(PoolGen * gen, void * ptr, PoolItem item);

void pool_gen_reset(PoolGen * gen, PoolItem item);

void       * pool_gen_mut(PoolGen       * gen, PoolId        id, PoolItem item);
void const * pool_gen_ref(PoolGen const * gen, PoolId        id, PoolItem item);
PoolId       pool_gen_id (PoolGen const * gen, void const * ptr, PoolItem item);

// |================================================================================================|
// |> POOL                                                                                          |
// |================================================================================================|

#define POOL(T) \
	union { \
		PoolGen as_gen; \
		struct { \
			PAGES_EMBED(ps, is_valid, beg, cur, pos, end); \
			union { T * free; T const * free_const; }; \
			T * null; \
		}; \
	}

#define pool_type(__pool__)     typeof(*(__pool__)->free)
#define pool_type_mut(__pool__) typeof( (__pool__)->free)
#define pool_type_ref(__pool__) typeof( (__pool__)->free_const)

#define pool_item(__pool__) pool_item_init(sizeof(pool_type(__pool__)))
#define pool_gen(__pool__) (&(__pool__)->as_gen)

#define pool_init(__pool__, __count__) pool_gen_init(pool_gen(__pool__), (__count__), pool_item(__pool__))
#define pool_deinit(__pool__) pool_gen_deinit(pool_gen(__pool__))

#define pool_alloc(__pool__) ((pool_type_mut(__pool__))pool_gen_alloc(pool_gen(__pool__), pool_item(__pool__)))
#define pool_free(__pool__, __ptr__) pool_gen_free(pool_gen(__pool__), (__ptr__), pool_item(__pool__))
#define pool_reset(__pool__) pool_gen_reset((__pool__), pool_item(__pool__))

#define pool_mut(__pool__, __id__) ((pool_type_mut(__pool__))pool_gen_mut(pool_gen(__pool__), (__id__), pool_item(__pool__)))
#define pool_ref(__pool__, __id__) ((pool_type_ref(__pool__))pool_gen_ref(pool_gen(__pool__), (__id__), pool_item(__pool__)))
#define pool_id(__pool__, __ptr__) pool_gen_id(pool_gen(__pool__), (__ptr__), pool_item(__pool__))

#endif // !STD_MEM_POOL_H
