#include "str/str.h"
#include "trc/assert.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// c-string

usz len_z(const u8 * s, usz limit) {
	for (usz i = 0; i < limit; i += 1) if (s[i] == '\0') return i;
	return limit;
};

////////////////////////////////////////////////////////////////////////////////
// string view

////////////////////////////////
// string invariant

bool str_sane(Str x) { return x.ptr != NULL; };

////////////////////////////////
// string copy

bool str_copy(StrMut dst, Str src) {
	ASSERT_DEBUG(str_sane(dst.ref), "invalid invariant");
	ASSERT_DEBUG(str_sane(src), "invalid invariant");
	if (dst.len < src.len) return FALSE;
	for (usz i = 0; i < src.len; i += 1) dst.ptr[i] = src.ptr[i];
	return TRUE;
};

////////////////////////////////
// string index

u8 str_at(Str x, usz i) {
	ASSERT_DEBUG(str_sane(x), "invalid invariant");
	ASSERT_DEBUG(i < x.len, "index out of bounds");
	return x.ptr[i];
};

Str str_cut_ex(CallLoc call, Str x, usz s, usz e) {
	ASSERT_DEBUG_AT(call, str_sane(x), "invalid invariant");
	ASSERT_DEBUG_AT(call, s <= e, "expected that [s] <= [e]");
	ASSERT_DEBUG_AT(call, e <= x.len, "cut out of bounds");
	return (Str){.ptr = x.ptr + s, .len = e - s};
};

Str str_sub(Str x, usz s, usz e) {
	ASSERT_DEBUG(str_sane(x), "invalid invariant");
	e = MIN(e, x.len);
	s = MIN(s, e);
	return (Str){.ptr = x.ptr + s, .len = e - s};
};

Str str_sub_win(Str x, usz s, usz e, usz r) {
	ASSERT_DEBUG(str_sane(x), "invalid invariant");
	s = (s > r) ? s - r : 0;
	e = e + r;
	return str_sub(x, s, e);
};

////////////////////////////////
// string comparison

bool str_eq(Str a, Str b) {
	ASSERT_DEBUG(str_sane(a) && str_sane(b), "invalid invariant");
	if (a.len != b.len) return FALSE;
	for (usz i = 0; i < a.len; i += 1) if (a.ptr[i] != b.ptr[i]) return FALSE;
	return TRUE;
};

i32 str_cmp(Str a, Str b) {
	ASSERT_DEBUG(str_sane(a) && str_sane(b), "invalid invariant");
	
	usz min = MIN(a.len, b.len);
	i32 cmp = memcmp(a.ptr, b.ptr, min);
	if (cmp != 0) return cmp;
	
	if (a.len > b.len) return +1;
	if (a.len < b.len) return -1;
	return 0;
};

bool str_prefix(Str a, Str b) {
	ASSERT_DEBUG(str_sane(a) && str_sane(b), "invalid invariant");
	return str_eq(str_sub(a, 0, b.len), b);
};

bool str_suffix(Str a, Str b) {
	ASSERT_DEBUG(str_sane(a) && str_sane(b), "invalid invariant");
	if (a.len < b.len) return FALSE;
	return str_eq(str_sub(a, a.len - b.len, a.len), b);
};

usz str_pos(Str src, Str sub) {
	ASSERT_DEBUG(str_sane(src) && str_sane(sub), "ivalid invariant");
	if (src.len < sub.len) return USZ_MAX;
	for (usz i = 0; i < src.len - sub.len; i += 1) {
		Str x = str_cut(src, i, i + sub.len);
		if (str_eq(x, sub)) return i;
	};
	return USZ_MAX;
};

////////////////////////////////
// slice of string slices

usz str_in(StrSlice items, Str item) {
	ASSERT_DEBUG(str_sane(item) && items.ptr != NULL, "invalid invariant");
	if (items.len == 0) return USZ_MAX;
	
	for (usz i = 0; i < items.len; i += 1) {
		if (str_eq(item, items.ptr[i])) return i;
	};

	return USZ_MAX;
};

////////////////////////////////
// string chop

#define STR_IS_CHOP(choped) ((choped).div.len != 0)

StrChoped str_chop(Str x, Str d) {
	ASSERT_DEBUG(str_sane(x) && str_sane(d), "invalid invariant");
	ASSERT(d.len > 0, "expected non-empty diveder");
	
	if (x.len < d.len) goto not_choped;
	
	for (usz i = 0; i < x.len - d.len; i += 1) {
		Str div = str_cut(x, i, i + d.len);
		if (str_eq(div, d)) {
			Str lhs = str_cut(x, 0, i);
			Str rhs = str_cut(x, i + d.len, x.len);
			return (StrChoped){lhs, div, rhs};
		};
	};
	
not_choped:
	return (StrChoped){x, STR(""), STR("")};
};

////////////////////////////////
// string location

////////////////////////////////////////////////////////////////////////////////
// ascii

typedef u8 ascii;

bool ascii_is_space(ascii b) {
	return ' ' == b || '\t' == b || '\r' == b || '\n' == b;
};

bool ascii_is_digit(ascii b) {
	return '0' <= b && b <= '9';
};

bool ascii_is_alpha(ascii b) {
	return ('A' <= b && b <= 'Z') || ('a' <= b && b <= 'z');
};

bool ascii_is_ident(ascii b) {
	return ascii_is_digit(b) || ascii_is_alpha(b);
};

Str ascii_trim_spaces(Str x) {
	ASSERT_DEBUG(str_sane(x), "invalid invariant");

	if (x.len == 0) return x;

	usz s = 0;
	for (;; s += 1) {
		if (s >= x.len) return str_cut(x, x.len - 1, x.len);
		if (!ascii_is_space(x.ptr[s])) break;
	};

	usz e = x.len - 1;
	for (;; e -= 1) {
		if (!ascii_is_space(x.ptr[e])) break;
	};

	return str_cut(x, s, e + 1);
};

StrLoc ascii_loc(Str src, usz at) {
	ASSERT_DEBUG(str_sane(src), "invalid invariant");
	src = str_sub(src, 0, at);

	StrLoc loc = (StrLoc){.row = 1, .col = 0};
	for (usz i = 0; i < src.len; i += 1) {
		if (src.ptr[i] == '\n') {
			loc.row += 1;
			loc.col = 0;
		} else {
			loc.col += 1;
		};
	};

	if (loc.col == 0) loc.col = 1;

	return loc;
};

u8 ascii_levenshtein_ex(Str a, Str b, u8 * tmp, usz len, CallLoc call) {
	if (a.len > b.len) { Str t = a; a = b; b = t; };
	if (a.len == 0) return (b.len > 255) ? 255 : (u8)b.len;
	
	ASSERT_AT(call, len >= 2 * 2 * (a.len + 1), "");
	u8 * prev = tmp;
	u8 * curr = tmp + (a.len + 1);

	for (usz i = 0; i <= a.len; i++) prev[i] = (u8)i;

	for (usz i = 1; i <= b.len; i++) {
		curr[0] = (u8)i;
		u8 min_in_row = curr[0];

		for (usz j = 1; j <= a.len; j++) {
			u8 cost = (a.ptr[j - 1] == b.ptr[i - 1]) ? 0 : 2;
			
			u8 sub = prev[j - 1] + cost;
			u8 del = prev[j] + 1;
			u8 ins = curr[j - 1] + 1;

			u8 res = sub;
			if (del < res) res = del;
			if (ins < res) res = ins;
			
			curr[j] = res;
			if (res < min_in_row) min_in_row = res;
		};

		if (min_in_row > ASCII_LEVENSHTEIN_THRESHOLD) return min_in_row;

		u8 *t = prev; prev = curr; curr = t;
	};

	return prev[a.len];
};

Str ascii_find_levenshtein_ex(Str x, StrSlice s, u8 * tmp, usz len, CallLoc call) {
	Str best = (Str){0};
	u8 min = 255;
	for (usz i = 0; i < s.len; i++) {
		u8 dist = ascii_levenshtein_ex(x, s.ptr[i], tmp, len, call);
		if (dist == 0) return s.ptr[i];
		if (dist < min) {
			min = dist;
			best = s.ptr[i];
		};
	};
	return best;
};

////////////////////////////////////////////////////////////////////////////////
// utf-8

usz utf8_char_len(u8 b) {
	if ((b & 0b10000000) == 0b00000000) return 1;
	if ((b & 0b11100000) == 0b11000000) return 2;
	if ((b & 0b11110000) == 0b11100000) return 3;
	if ((b & 0b11111000) == 0b11110000) return 4;
	return 0;
};

u32 utf8_next(Str * s) {
	if (s->ptr == NULL || s->len == 0) return 0;
	usz len = utf8_char_len(s->ptr[0]);
	u32 value = 0;
	u8 * buf = (void*)&value;

	usz eat = MIN(len, s->len);
	for (usz i = 0; i < eat; i += 1) buf[i] = s->ptr[i];
	s->ptr += eat; s->len -= eat;

	return value;
};

usz utf8_char_count(Str src) {
	usz i = 0;
	usz c = 0;
	while (i < src.len) {
		usz len = utf8_char_len(src.ptr[i]);
		if (len == 0) i += 1;
		else          i += len;
		c += 1;
	};
	return c;
};

StrLoc utf8_loc(Str src, usz at) {
	ASSERT_DEBUG(str_sane(src), "invalid invariant");
	src = str_sub(src, 0, at);

	StrLoc loc = (StrLoc){.row = 1, .col = 0};
	
	usz i = 0;
	while (i < src.len) {
		if (src.ptr[i] == '\n') {
			loc.row += 1;
			loc.col = 0;
		} else {
			loc.col += 1;
		};

		usz len = utf8_char_len(src.ptr[i]);
		if (len == 0) i += 1;
		else          i += len;
	};

	if (loc.col == 0) loc.col = 1;
	return loc;
};

