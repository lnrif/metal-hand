#include "std/mem/list.h"

void * list_gen_reserve(ListGen * list, u64 size, u32 align, u64 count) {
	if (list->cap - list->len >= count) return &list->raw[list->len * size];

	u64 const total_count = MAX((list->len == 0) ? 4 : list->len * 2, list->len + count);
	RegUpd const upd = reg_upd_arr_ex(size, align, list->raw, list->len, total_count, REG_DIR_UP);

	Reg const next = reg_upd(list->man, upd);
	if (!next.is_valid) return 0;

	list->ptr = next.ptr;
	list->cap = next.len / size;
	list->bytes = next.len;

	return &list->raw[list->len * size];
};

