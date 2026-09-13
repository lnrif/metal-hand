#ifndef RK_STR_H
#define RK_STR_H

#include "core/types.h"

////////////////////////////////////////////////////////////////
// c-string

// return len of str-z with [limit]
u64 len_z(const u8 * s, u64 limit);

////////////////////////////////////////////////////////////////
// string view

#define STR_FIELDS(ptr, len) \
	u8 const * ptr; \
	u64 len

#define STR_EMBED(s, ptr, len) \
	union { \
		Str s; \
		struct { STR_FIELDS(ptr, len); }; \
	};

typedef struct {
	STR_FIELDS(ptr, len);
} Str;

typedef union {
	Str ref;
	struct { u8 * ptr; u64 len; };
} StrMut;

typedef Str StrZ;
typedef Str StrOpt;

#define STR_NONE      ((Str){.ptr = NULL, .len = 0})
#define STR_MUT_NONE  ((StrMut){.ptr = NULL, .len = 0})
#define STR(lit)    ((Str){.ptr = (u8*)(void*)( lit), .len = sizeof(lit) - 1       })
#define STR_Z(_ptr) ((Str){.ptr = (u8*)(void*)(_ptr), .len = len_z((u8*)(void*)(_ptr), USZ_MAX)})

////////////////////////////////
// string invariant

b8 str_sane(Str x);

////////////////////////////////
// string copy

b8 str_copy(StrMut dst, Str src);

////////////////////////////////
// string index

u8  str_at(Str x, u64 i);

#define str_cut(x, s, e) str_cut_ex(CALL, x, s, e)
Str str_cut_ex(CallLoc call, Str x, u64 s, u64 e);

Str str_sub(Str x, u64 s, u64 e);

//       s         e
// |-----|---------|-----|
// |  r  |  e - s  |  r  |
Str str_sub_win(Str x, u64 s, u64 e, u64 r);

////////////////////////////////
// string comparison

b8 str_eq(Str a, Str b);
i32  str_cmp(Str a, Str b);
b8 str_prefix(Str a, Str b);
b8 str_suffix(Str a, Str b);
u64  str_pos(Str src, Str sub);

#define str_eq_lit(x, lit) str_eq(x, STR(lit))

////////////////////////////////
// slice of string slices

typedef struct {
	Str const * ptr;
	u64 len;
} StrSlice;

u64 str_in(StrSlice items, Str item);

////////////////////////////////
// string chop

typedef struct {
	Str lhs;
	Str div;
	Str rhs;
} StrChoped;

#define STR_IS_CHOP(choped) ((choped).div.len != 0)

StrChoped str_chop(Str x, Str d);

////////////////////////////////
// string location

typedef struct {
	// zero based, invalid [U32_MAX]
	u32 row;
	// zero based, invalid [U32_MAX]
	u32 col;
} StrLoc;

////////////////////////////////////////////////////////////////////////////////
// ascii

b8 ascii_is_space(u8 b);
b8 ascii_is_digit(u8 b);
b8 ascii_is_alpha(u8 b);
b8 ascii_is_ident(u8 b);

Str    ascii_trim_spaces(Str x);
StrLoc ascii_loc(Str s, u64 at);

#define ASCII_LEVENSHTEIN_THRESHOLD 32
#define ascii_levenshtein(a, b, tmp, len) ascii_levenshtein_ex(a, b, tmp, len, CALL)

// assert: [len] == 2 * (min(a.len, b.len) + 1)
u8 ascii_levenshtein_ex(Str a, Str b, u8 * tmp, u64 len, CallLoc call);

#define ascii_find_levenshtein(x, s, tmp, len) ascii_find_levenshtein_ex(x, s, tmp, len, CALL)
Str ascii_find_levenshtein_ex(Str x, StrSlice s, u8 * tmp, u64 len, CallLoc call);

////////////////////////////////////////////////////////////////////////////////
// utf-8

u64 utf8_char_count(Str s);
u64 utf8_char_len(u8 b);
u32 utf8_next(Str * s);

// static u64 utf8_levenshtein(Str a, Str b, u8 * tmp, u64 len);

StrLoc utf8_loc(Str s, u64 at);

#endif // !RK_STR_H
