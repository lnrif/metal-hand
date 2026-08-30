#ifndef STD_STR_H
#define STD_STR_H

// |================================================================================================|
// |> STRING                                                                                        |
// |================================================================================================|

#include "std/core.h"

typedef struct {
	u8 const * ptr;
	u64 len;
} Str;

typedef union {
	Str str;
	struct {
		u8 * ptr;
		u64 len;
	};
} StrMut;

typedef union {
	Str str;
	struct {
		union {
			u8 const * ptr_z;
			u8 const * ptr;
		};
		// NOT INCLUDE '\0'
		u64 len;
	};
} StrZ;

#define STR_NONE     ((Str   ){.ptr = 0, .len = 0})
#define STR_Z_NONE   ((StrZ  ){.ptr = 0, .len = 0})
#define STR_MUT_NONE ((StrMut){.ptr = 0, .len = 0})

StrZ str_z_init(u8z const * ptr_z);

#define str_is_none(x) ((x)->ptr == 0)

#define STR(_ptr, _len)     ((Str ){.ptr   = (_ptr  ), .len = (_len)})
#define STR_Z(_ptr_z, _len) ((StrZ){.ptr_z = (_ptr_z), .len = (_len)})

#define S(lit) ((Str ){.ptr = (u8*)(void*)(lit), .len = sizeof(lit) - 1})
#define Z(lit) ((StrZ){.ptr = (u8*)(void*)(lit), .len = sizeof(lit) - 1})

// |================================================================================================|
// |> [Str]: find                                                                                   |

u8 * str_find_byte(Str s, u8 b);

// |================================================================================================|
// |> [Str]: comparision                                                                            |

b8 str_eq(Str a, Str b);
#define str_eq_lit(x, lit) str_eq(x, S(lit))

#endif // !STD_STR_H
