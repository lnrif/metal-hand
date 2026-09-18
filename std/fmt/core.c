#include "std/fmt/core.h"
#include "std/mem/reg.h"
#include "std/str/core.h"
#include "std/mem/core.h"
#include "std/flow/core.h"

// |================================================================================================|
// |> [Fmt]: constructors                                                                        |

Fmt fmt_init(RegMan man, FmtMask flow) {
	return (Fmt){
		.reg = REG_NIL,
		.pos = 0,
		.man = man,
		.last = STR_MUT_NIL,
		.flow = flow,
	};
};

Fmt fmt_from_slice(void * ptr, u64 len, FmtMask flow) {
	return (Fmt){
		.ptr = (uptr)ptr, .len = len,
		.pos = 0, .man = REG_MAN_NIL,
		.last = STR_MUT_NIL, .flow = flow,
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
		.flow = out->flow,
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
	if (FMT_S_IS_ESCAPE(style->opt)) {
		str_len = 0;
		for (u32 i = 0; i < src.len; i += 1) switch (src.raw[i]) {
			case '"': case '\e': case '\t': case '\r': case '\n': case '\0': str_len += 2; break;
			default: str_len += (src.raw[i] < 32) ? 4 : 1;                                 break;
		};
	};

	u64 full_len = str_len;
	if (FMT_S_IS_QUOTES(style->opt)) full_len += 2;

	if (full_len < style->width) full_len = style->width;
	return (FmtStrShot){.full_len = full_len, .str_len = str_len};
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
	if (!fmt_reserve(out, shot.full_len)) return false;

	u8 * ptr = (u8*)out->ptr + out->pos;
	u64 const offset = FMT_S_IS_QUOTES(style->opt) ? 1 : 0;

	// TODO: add mid align
	u8 * space = FMT_IS_RHS(style->opt) ? &ptr[0] : &ptr[shot.str_len + offset * 2];
	u64 spaces_count = shot.full_len - shot.str_len - offset * 2;
	memset(space, style->fill, spaces_count);

	u8 * write = FMT_IS_RHS(style->opt) ? &ptr[shot.full_len - shot.str_len - offset * 2] : &ptr[0];

	if (FMT_S_IS_QUOTES(style->opt)) {
		write[0] = '"'; write += 1;
	};

	if (FMT_S_IS_ESCAPE(style->opt)) {
		fmt_str_escaped(write, src);
	} else {
		memcpy(write, src.raw, src.len);
	};

	write += shot.str_len;

	if (FMT_S_IS_QUOTES(style->opt)) {
		write[0] = '"'; write += 1;
	};

	out->pos += shot.full_len;
	out->last = (StrMut){.raw = ptr, .len = shot.full_len};

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

	u64 base;
	switch (FMT_N_GET_BASE(style->opt)) {
		case FMT_N_BIN: base =  2; break;
		case FMT_N_OCT: base =  8; break;
		case FMT_N_DEC: base = 10; break;
		case FMT_N_HEX: base = 16; break;
	};

	u64 num_len = 0; for (;;) {
		num_len += 1;
		src /= base;
		if (src == 0) break;
	};

	u64 const digits_len = num_len > style->digits ? num_len : style->digits;

	u64 field_len = digits_len;
	if (neg || FMT_N_IS_SIGN(style->opt)) field_len += 1;
	if (base != 10 && FMT_N_IS_PREFIX(style->opt)) field_len += 2;

	u64 full_len = field_len > style->width ? field_len : style->width;
	
	return (FmtNumShot){
		.full_len = full_len,
		.field_len = field_len,
		.digits_len = digits_len,
	};
};

b8 fmt_num_write(Fmt * out, u64 src, b8 neg, FmtNumStyle const * style, FmtNumShot shot) {
	if (!fmt_reserve(out, shot.full_len)) return false;

	// TODO: add mid align
	u8 * ptr = (u8 *)out->ptr + out->pos;
	u8 * space = FMT_IS_RHS(style->opt) ? &ptr[0] : &ptr[shot.field_len];
	memset(space, style->fill, shot.full_len - shot.field_len);

	u8 * write = FMT_IS_RHS(style->opt) ? &ptr[shot.full_len - shot.field_len] : &ptr[0];
	u64 w = 0;

	if (FMT_N_IS_SIGN(style->opt)) {
		write[w] = neg ? '-' : (src == 0 ? style->fill : '+'); w += 1;
	} else if (neg) {
		write[w] = '-'; w += 1;
	};

	if (FMT_N_IS_PREFIX(style->opt)) switch (FMT_N_GET_BASE(style->opt)) {
		case FMT_N_BIN: {
			write[w] = '0'; w += 1;
			write[w] = 'b'; w += 1;
		} break;
		case FMT_N_OCT: {
			write[w] = '0'; w += 1;
			write[w] = 'o'; w += 1;
		} break;
		case FMT_N_DEC: break;
		case FMT_N_HEX: {
			write[w] = '0'; w += 1;
			write[w] = 'x'; w += 1;
		} break;
	};

	u64 base;
	switch (FMT_N_GET_BASE(style->opt)) {
		case FMT_N_BIN: base =  2; break;
		case FMT_N_OCT: base =  8; break;
		case FMT_N_DEC: base = 10; break;
		case FMT_N_HEX: base = 16; break;
	};

	u8 buf[sizeof(u64) * 8] = {0};
	u8 num_idx = sizeof(buf);
	u8 const * const table = (u8*)"0123456789ABCDEF";

	for (;;) {
		num_idx -= 1;
		buf[num_idx] = table[src % base];
		src /= base;
		if (src == 0) break;
	};

	u64 const num_len = sizeof(buf) - num_idx;
	u64 const zeros = shot.digits_len - num_len;
	memset(&write[w], '0', zeros); w += zeros;
	memcpy(&write[w], &buf[num_idx], num_len);

	out->pos += shot.full_len;
	out->last = (StrMut){.raw = ptr, .len = shot.full_len};

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

// |================================================================================================|
// |> Arguments Formatting                                                                          |
// |================================================================================================|

b8 fmt_write(Fmt * fmt, FmtArg * args, u64 len) {
	StrMut str = STR_MUT(fmt->ptr + fmt->pos, 0);

	for (u64 i = 0; i < len; i += 1) {
		FmtArg arg = args[i];
		FmtMask flow = 1 << FMT_TAG_FLOW(arg.tag);
		
		// fmt_lit(fmt, "[");
		// fmt_u64(fmt, fmt->flow & 0b11, .opt = FMT_N_BIN | FMT_N_NO_PREFIX, .digits = 2);
		// fmt_lit(fmt, ":");
		// fmt_u64(fmt, flow, .opt = FMT_N_BIN | FMT_N_NO_PREFIX, .digits = 2);
		// fmt_lit(fmt, "]");
		
		if ((fmt->flow & flow) != flow) continue;

		b8 ok = true;
		switch (FMT_TAG_TY(arg.tag)) {
			case FMT_TY_STR: ok = fmt_str_ex(
				fmt,
				arg.as.str.src,
				&arg.as.str.style
			); break;
			case FMT_TY_U64: ok = fmt_u64_ex(
				fmt,
				arg.as.u64.src,
				&arg.as.u64.style
			); break;
			case FMT_TY_I64: ok = fmt_i64_ex(
				fmt,
				arg.as.i64.src,
				&arg.as.i64.style
			); break;

			// case FMT_ARG_MEM:                   result = fmt_virt_mem(fmt, arg.as.mem, arg.opt, NULL);   break;
			// case FMT_ARG_COLOR: if (fmt->color) result = fmt_virt_str(fmt, arg.as.color, arg.opt, NULL); break;
			default: PANIC("invalid [arg.tag]");
		};

		if (!ok) return false;
	};

	str.len = fmt->pos - (uptr)str.ptr;
	fmt->last = str;

	return true;
};

