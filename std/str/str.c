#include "std/str/str.h"
#include "std/mem/core.h"

// |================================================================================================|
// |> [Str]: find                                                                                   |

u8 * str_find_byte(Str s, u8 b) {
	return memchr(s.ptr, s.len, b);
};

// |================================================================================================|
// |> [Str]: comparision                                                                            |

b8 str_eq(Str a, Str b) {
	if (a.len != b.len) return false;
	return memcmp(a.ptr, b.ptr, a.len) == 0;
};

// |================================================================================================|
// |> [StrZ]: init                                                                                  |

StrZ str_z_init(u8z const * ptr_z) {
	return str_z_limit(ptr_z, U64_MAX);
};

StrZ str_z_limit(u8z const * ptr_z, u64 limit) {
	u8 const * at = memchr(ptr_z, limit, 0);
	if (at == 0) at = ptr_z + limit;
	return STR_Z(ptr_z, (u64)(ptr_z - at));
};
