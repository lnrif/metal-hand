#ifndef STD_LIST_CORE_H
#define STD_LIST_CORE_H

#include "std/core.h"
#include "std/mem/reg.h"
#include "std/mem/page.h"
#include "std/mem/core.h"

// |================================================================================================|
// |> LIST GENERIC                                                                                  |
// |================================================================================================|

typedef struct {
	union { uptr ptr; u8 * raw; };
	u64 len; u64 cap; u64 bytes;
	RegMan man;
} ListGen;

#define LIST_GEN_EMPTY(_man) ((ListGen){.len = 0, .cap = 0, .bytes = 0, .man = (_man)})

// b8 list_gen_init(ListGen * list, u64 size, u64 align, RegMan man);

void * list_gen_reserve(ListGen * list, u64 size, u32 align, u64 count);

// |================================================================================================|
// |> MACRO API                                                                                     |
// |================================================================================================|

#define LIST(T) \
	union { \
		ListGen as_gen; \
		struct { \
			union { T * ptr; u8 * raw; b64 is_valid; }; \
			u64 len; u64 cap; u64 _; \
			RegMan man; \
		}; \
	}

#define LIST_EMPTY(List, man) ((List){.as_gen = LIST_GEN_EMPTY(man)})

// #define list_init(list, man) \
// 	list_gen_init( \
// 		&(list)->as_gen, \
// 		sizeof((list)->ptr[0]), \
// 		alignof((list)->ptr[0]), \
// 		(man) \
// 	)

#define list_extend(__list__, __ptr__, __len__) \
	({ \
		u64 const std_list_extend_len = (__len__); \
		typeof((__list__)->ptr) _list_dst = list_gen_reserve( \
			&(__list__)->as_gen, \
			sizeof((__list__)->ptr[0]), \
			alignof(typeof((__list__)->ptr[0])), \
			std_list_extend_len \
		); \
		if (_list_dst != 0) { \
			memcpy(_list_dst, (__ptr__), std_list_extend_len * sizeof((__list__)->ptr[0])); \
			(__list__)->len += std_list_extend_len; \
		}; \
		_list_dst; \
	})

#define list_push(__list__, __v0__, __vs__...) \
	({ \
		typeof((__list__)->ptr[0]) const _list_values[] = {__v0__, __vs__}; \
		list_extend((__list__), _list_values, LEN(_list_values)); \
	})

#define list_pop(__list__) \
	({ \
		typeof((__list__)->ptr) _list_pop_item_ref = 0; \
		if ((__list__)->len > 0) { \
			(__list__)->len -= 1; \
			_list_pop_item_ref = &(__list__)->ptr[(__list__)->len]; \
		}; \
		_list_pop_item_ref; \
	})

#define list_reset(__list__) do { (__list__)->len = 0; } while (0) \

// void f(void) {
// 	typedef LIST_TYPE(u64) Nums;
//
// 	Pages pages = pages_reserve(MB(1));
// 	if (!pages.is_valid) return;
//
// 	Nums nums = LIST_EMPTY(Nums, pages_pinned(&pages));
// 	list_push(&nums, 1, 2, 3);
//
// 	for (u64 i = 0; i < nums.len; i += 1) {
// 		u64 x = nums.ptr[i]; UNUSED(x);
// 		// ...
// 	};
//
// 	list_pop(&nums);
//
// };

#endif // !STD_LIST_CORE_H
