#include "std/str/core.h"
#include "std/mem/core.h"
#include "std/flow/core.h"

// |================================================================================================|
// |> [Str]: find                                                                                   |

u8 * str_find_byte(Str s, u8 b) {
	return memchr(s.raw, s.len, b);
};

// |================================================================================================|
// |> [Str]: index                                                                                  |

u8 str_idx(Str x, u64 i) {
	ASSERT(str_sane(x), "invalid invariant");
	ASSERT(i < x.len, "index out of bounds");
	return x.raw[i];
};

Str str_cut(Str x, u64 s, u64 e) {
	ASSERT(str_sane(x), "invalid invariant");
	ASSERT(s <= e, "expected that [s] <= [e]");
	ASSERT(e <= x.len, "cut out of bounds");
	return (Str){.ptr = x.ptr + s, .len = e - s};
};

Str str_sub(Str x, u64 s, u64 e) {
	ASSERT(str_sane(x), "invalid invariant");
	e = MIN(e, x.len); s = MIN(s, e);
	return (Str){.ptr = x.ptr + s, .len = e - s};
};

// |================================================================================================|
// |> [Str]: comparision                                                                            |

b8 str_eq(Str a, Str b) {
	if (a.len != b.len) return false;
	for (u64 i = 0; i < a.len; i += 1) if (a.raw[i] != b.raw[i]) return false;
	return true;
};

i32 str_cmp(Str a, Str b) {
	u64 min = MIN(a.len, b.len);
	i32 cmp = memcmp(a.raw, b.raw, min);
	if (cmp != 0) return cmp;
	if (a.len > b.len) return +1;
	if (a.len < b.len) return -1;
	return 0;
};

Str str_pos(Str a, Str b) {
	ASSERT(str_sane(a) && str_sane(b), "ivalid invariant");
	if (a.len < b.len) return STR_NIL;
	for (u64 i = 0; i < a.len - b.len; i += 1) {
		Str x = str_cut(a, i, i + b.len);
		if (str_eq(x, b)) return x;
	};
	return STR_NIL;
};

b8 str_beg(Str a, Str b) {
	if (a.len < b.len) return false;
	return str_eq(str_sub(a, 0, b.len), b);
};

b8 str_end(Str a, Str b) {
	if (a.len < b.len) return false;
	return str_eq(str_sub(a, a.len - b.len, a.len), b);
};

// |================================================================================================|
// |> [StrZ]: init                                                                                  |

StrZ str_z_init(u8z const * ptr_z) {
	return str_z_limit(ptr_z, U64_MAX);
};

StrZ str_z_limit(u8z const * ptr_z, u64 limit) {
	u8 const * at = memchr(ptr_z, limit, 0);
	if (at == 0) at = ptr_z + limit;
	return STR_Z(ptr_z, (u64)(at - ptr_z));
};
