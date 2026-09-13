#include "std/fmt/core.h"
#include "std/mem/reg.h"
#include "std/str/str.h"
#include "std/mem/core.h"

// |================================================================================================|
// |> [Fmt]: constructors                                                                        |

Fmt fmt_init(RegMan man) {
	return (Fmt){.reg = REG_NIL, .pos = 0, .man = man, .last = STR_MUT_NIL};
};

Fmt fmt_from_slice(void * ptr, u64 len) {
	return (Fmt){
		.ptr = (uptr)ptr, .len = len,
		.pos = 0, .man = REG_MAN_NIL,
		.last = STR_MUT_NIL,
	};
};

// |================================================================================================|
// |> [Fmt]: mark and load                                                                       |

void fmt_reset(Fmt * out) {
	out->last = STR_MUT_NIL;
	out->pos = 0;
};

// |================================================================================================|
// |> [Fmt]: helpers                                                                             |

void fmt_rebase(Fmt * out, Reg reg) {
	*out = (Fmt){
		.reg = reg,
		.last = (StrMut){
			.ptr = reg.ptr + (out->last.ptr - out->reg.ptr),
			.len = out->last.len,
		},
		.pos = out->pos,
		.man = out->man,
	};
};

b8 fmt_reserve(Fmt * out, u64 add) {
	u64 const rem = out->len - out->pos;
	if (rem >= add) return true;

	Reg new = reg_upd_arr(out->man, out->raw, out->len, out->len + (add - rem), REG_DIR_UP);
	if (!new.is_valid) return false;

	fmt_rebase(out, new);
	return true;
};

b8 fmt_flush_file(Fmt * fmt, u8z const * path) {
	if (!fs_write_all(path, fmt->any, fmt->pos)) return false;
	fmt_reset(fmt); return true;
};

b8 fmt_flush_stream(Fmt * fmt, Stream * stream) {
	if (stream_write_str(stream, fmt_as_str(fmt)) < 0) return false;
	fmt_reset(fmt); return true;
};

Str fmt_as_str(Fmt const * out) {
	return (Str){.ptr = out->ptr, .len = out->pos};
};

// |================================================================================================|
// |> Raw Formatting                                                                                |
// |================================================================================================|

b8 fmt_lit_ex(Fmt * out, Str str) {
	if (!fmt_reserve(out, str.len)) return false;

	u8 * const ptr = (u8 *)out->reg.ptr + out->pos;
	memcpy(ptr, str.raw, str.len);

	out->pos += str.len;
	out->last = (StrMut){.raw = ptr, .len = str.len};
	return true;
};

// |================================================================================================|
// |> String Formatting                                                                             |
// |================================================================================================|

FmtStrShot fmt_str_shot(Str src, FmtStrStyle * style) {
	if (style->fill == 0) style->fill = ' ';

	u64 str_len = src.len;
	if (style->escape) {
		str_len = 0;
		for (u32 i = 0; i < src.len; i += 1) switch (src.raw[i]) {
			case '"': case '\e': case '\t': case '\r': case '\n': case '\0': str_len += 2; break;
			default: str_len += (src.raw[i] < 32) ? 4 : 1;                      break;
		};
	};

	u64 len = str_len;
	if (style->quotes) len += 2;

	if (len < style->width) len = style->width;
	return (FmtStrShot){.len = len, .str_len = str_len};
};

static void fmt_str_escaped(u8 * write, Str src) {
	u64 w = 0;
	for (u64 r = 0; r < src.len; r += 1) {
		u8 const b = src.raw[r];
		switch (b) {
			case '"': case '\e': case '\t': case '\r': case '\n': case '\0': {
				write[w] = '\\'; w += 1;
				switch (b) {
					case '"':  write[w] = '"'; break;
					case '\e': write[w] = 'e'; break;
					case '\t': write[w] = 't'; break;
					case '\r': write[w] = 'r'; break;
					case '\n': write[w] = 'n'; break;
					case '\0': write[w] = '0'; break;
				}; w += 1;
			} break;
			default: {
				if (b >= 32) {
					write[w] = b; w += 1;
					break;
				};
				
				u8 const * const table = (u8*)"0123456789ABCDEF";
				write[w] = '\\'; w += 1;
				write[w] = 'x'; w += 1;
				write[w] = table[(b >> 4) & 0xF]; w += 1;
				write[w] = table[(b >> 0) & 0xF]; w += 1;
			} break;
		};
	};
};

b8 fmt_str_write(Fmt * out, Str src, FmtStrStyle const * style, FmtStrShot shot) {
	if (!fmt_reserve(out, shot.len)) return false;

	u8 * ptr = (u8*)out->ptr + out->pos;
	u64 const offset = style->quotes ? 1 : 0;

	u8 * space = style->right ? &ptr[0] : &ptr[shot.str_len + offset * 2];
	u64 spaces_count = shot.len - shot.str_len - offset;
	memset(space, style->fill, spaces_count);

	u8 * write = style->right ? &ptr[shot.len - shot.str_len - offset * 2] : &ptr[offset];

	if (style->quotes) {
		write[0] = '"';
		write += 1;
		write[shot.str_len] = '"';
	};

	if (style->escape) {
		fmt_str_escaped(write, src);
	} else {
		memcpy(write, src.raw, src.len);
	};

	if (style->quotes) write[shot.str_len] = '"';

	out->pos += shot.len;
	out->last = (StrMut){.raw = ptr, .len = shot.len};
	return true;
};

b8 fmt_str_ex(Fmt * out, Str src, FmtStrStyle * style) {
	return fmt_str_write(out, src, style, fmt_str_shot(src, style));
};

// |================================================================================================|
// |> Number Formatting                                                                             |
// |================================================================================================|

FmtNumShot fmt_num_shot(u64 src, b8 neg, FmtNumStyle * style) {
	if (style->fill == 0) style->fill = ' ';
	if (style->base == 0) style->base = 10;

	if (style->base != 2 && style->base != 8 && style->base != 10 && style->base != 16) {
		// TODO: add error shot value
		return (FmtNumShot){0};
	};

	u64 num_len = 0; for (;;) {
		num_len += 1;
		src /= style->base;
		if (src == 0) break;
	};
	u64 const digits_len = num_len > style->digits ? num_len : style->digits;

	u64 field_len = digits_len;
	if (neg || style->sign)                field_len += 1;
	if (style->prefix && style->base != 10) field_len += 2;

	u64 len = field_len > style->width ? field_len : style->width;
	
	return (FmtNumShot){
		.len = len,
		.field_len = field_len,
		.digits_len = digits_len,
	};

};

b8 fmt_num_write(Fmt * out, u64 src, b8 neg, FmtNumStyle const * style, FmtNumShot shot) {
	if (!fmt_reserve(out, shot.len)) return false;

	u8 * ptr = (u8 *)out->ptr + out->pos;
	u8 * space = style->right ? &ptr[0] : &ptr[shot.field_len];
	memset(space, style->fill, shot.len - shot.field_len);

	u8 * write = style->right ? &ptr[shot.len - shot.field_len] : &ptr[0];
	u64 w = 0;

	if (style->sign) {
		write[w] = neg ? '-' : (src == 0 ? style->fill : '+'); w += 1;
	} else if (neg) {
		write[w] = '-'; w += 1;
	};

	if (style->prefix) switch (style->base) {
		case 2: {
			write[w] = '0'; w += 1;
			write[w] = 'b'; w += 1;
		} break;
		case 8: {
			write[w] = '0'; w += 1;
			write[w] = 'o'; w += 1;
		} break;
		case 10: break;
		case 16: {
			write[w] = '0'; w += 1;
			write[w] = 'x'; w += 1;
		} break;
	};

	u8 buf[64] = {0};
	u8 num_idx = sizeof(buf);
	u8 const * const table = (u8*)"0123456789ABCDEF";

	for (;;) {
		num_idx -= 1;
		buf[num_idx] = table[src % style->base];
		src /= style->base;
		if (src == 0) break;
	};

	u64 const num_len = sizeof(buf) - num_idx;
	u64 const zeros = shot.digits_len - num_len;
	memset(&write[w], '0', zeros); w += zeros;
	memcpy(&write[w], &buf[num_idx], num_len);

	out->pos += shot.len;
	out->last = (StrMut){.raw = ptr, .len = shot.len};
	return true;
};

b8 fmt_num_ex(Fmt * out, u64 src, u8 neg, FmtNumStyle * style) {
	return fmt_num_write(out, src, neg, style, fmt_num_shot(src, neg, style));
};

// |================================================================================================|
// |> Number Formatting: u64                                                                        |
// |================================================================================================|

// ...

// |================================================================================================|
// |> Number Formatting: i64                                                                        |
// |================================================================================================|

b8 fmt_i64_ex(Fmt * out, i64 src, FmtNumStyle * style) {
	return fmt_num_ex(out, src < 0 ? (u64)-src : (u64)src, src < 0, style);
};

