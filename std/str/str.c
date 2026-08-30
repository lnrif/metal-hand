#include "std/str/str.h"
#include "std/mem/core.h"

// |================================================================================================|
// |> [Str]: find                                                                                   |

u8 * str_find_byte(Str s, u8 b) {
	return memchr(s.ptr, s.len, b);
};

u64 len_z(u8 const * s, u64 limit) {
	u8 const * at = memchr(s, limit, 0);
	if (at == 0) return limit;
	return (u64)(at - s);
};

StrZ str_z_init(u8z const * ptr_z) {
	return STR_Z(ptr_z, (u64)ptr_z - (u64)memchr(ptr_z, U64_MAX, 0));
};

b8 str_is_none_ex(Str const * str) {
	return str->ptr == 0;
};

// |================================================================================================|
// |> [Str]: comparision                                                                            |

b8 str_eq(Str a, Str b) {
	if (a.len != b.len) return false;
	return memcmp(a.ptr, b.ptr, a.len) == 0;
};

