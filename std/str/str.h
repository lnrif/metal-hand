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

#define STR(_ptr, _len) ((Str){.ptr = (_ptr), .len = (_len)})
#define STR_NIL STR(0, 0)
#define S(lit)  STR((u8*)(void*)(lit), sizeof(lit) - 1)

#define STR_MUT(_ptr, _len) ((StrMut){.ptr = (_ptr), .len = (_len)})
#define STR_MUT_NIL STR_MUT(0, 0)

// |================================================================================================|
// |> [Str]: find                                                                                   |

u8 * str_find_byte(Str s, u8 b);

// |================================================================================================|
// |> [Str]: comparision                                                                            |

b8 str_eq(Str a, Str b);
#define str_eq_lit(x, lit) str_eq(x, S(lit))

// |================================================================================================|
// |> [Str]: properties                                                                             |

#define str_is_nil(x) ((x)->ptr == 0)

// |================================================================================================|
// |> C-STRING                                                                                      |
// |================================================================================================|

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

// |================================================================================================|
// |> [StrZ]: init                                                                                  |

#define STR_Z(_ptr_z, _len) ((StrZ){.ptr = (_ptr_z), .len = (_len)})

#define STR_Z_NIL STR_Z(0, 0)
#define Z(lit) STR_Z((u8*)(void*)(lit), sizeof(lit) - 1)

StrZ str_z_init(u8z const * ptr_z);
StrZ str_z_limit(u8z const * ptr_z, u64 limit);

#endif // !STD_STR_H
