#include "std/fmt/core.h"
#include "std/mem/reg.h"
#include "std/str/core.h"
#include "std/mem/core.h"
#include "std/flow/core.h"
#include "std/fs/core.h"

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

	u64 field_len = str_len;
	if (FMT_S_IS_QUOTES(style->opt)) field_len += 2;
	u64 full_len = MAX(field_len, style->width);

	return (FmtStrShot){
		.full_len = full_len,
		.field_len = field_len,
		.str_len = str_len,
	};
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

typedef struct {
	u8 * block;
	u8 * field;
	u64 full_len;
} FmtLayout;

static inline FmtLayout fmt_layout(Fmt * out, u64 field_len, u32 width, u8 align, u8 fill) {
	u64 full_len = field_len > width ? field_len : width;
	if (!fmt_reserve(out, full_len)) return (FmtLayout){.block = 0, .field = 0, .full_len = 0};

	u8 * ptr = (u8 *)out->ptr + out->pos;
	u8 * field = ptr;
	u64 pad_total = full_len - field_len;

	if (pad_total > 0) {
		u64 pad_lhs = 0;
		u64 pad_rhs = 0;

		switch (align) {
			case FMT_LHS: {
				pad_lhs = 0;
				pad_rhs = pad_total;
				field = ptr;
			} break;
			case FMT_RHS: {
				pad_lhs = pad_total;
				pad_rhs = 0;
				field = &ptr[pad_lhs];
			} break;
			case FMT_MID: {
				pad_lhs = pad_total / 2;
				pad_rhs = pad_total - pad_lhs;
				field = &ptr[pad_lhs];
			} break;
			default: PANIC("invalid [align] in fmt_layout");
		};

		if (pad_lhs > 0) memset(ptr, fill, pad_lhs);
		if (pad_rhs > 0) memset(&ptr[pad_lhs + field_len], fill, pad_rhs);
	};

	return (FmtLayout){.block = ptr, .field = field, .full_len = full_len};
};

b8 fmt_str_write(Fmt * out, Str src, FmtStrStyle const * style, FmtStrShot shot) {
	FmtLayout layout = fmt_layout(out, shot.field_len, style->width, FMT_GET_ALIGN(style->opt), style->fill);
	if (!layout.block) return false;

	u8 * w = layout.field;

	if (FMT_S_IS_QUOTES(style->opt)) { w[0] = '"'; w += 1; };

	if (FMT_S_IS_ESCAPE(style->opt)) {
		fmt_str_escaped(w, src); 
	} else {
		memcpy(w, src.raw, src.len);
	}

	w += shot.str_len;
	if (FMT_S_IS_QUOTES(style->opt)) { w[0] = '"'; w += 1; };

	out->pos += layout.full_len;
	out->last = (StrMut){.raw = layout.block, .len = layout.full_len};

	return true;
};

// b8 fmt_str_write(Fmt * out, Str src, FmtStrStyle const * style, FmtStrShot shot) {
// 	if (!fmt_reserve(out, shot.full_len)) return false;
//
// 	u8 * ptr = (u8*)out->ptr + out->pos;
// 	u8 * space_lhs; u64 space_lhs_len;
// 	u8 * space_rhs; u64 space_rhs_len;
//
// 	switch (FMT_GET_ALIGN(style->opt)) {
// 		case FMT_LHS: {
// 			space_lhs = 0;
// 			space_lhs_len = 0;
// 			space_rhs = &ptr[shot.field_len];
// 			space_rhs_len = shot.full_len - shot.field_len;
// 		}; break;
//
// 		case FMT_RHS: {
// 			space_lhs = &ptr[0];
// 			space_lhs_len = shot.full_len - shot.field_len;
// 			space_rhs = 0;
// 			space_rhs_len = 0;
// 		}; break;
//
// 		case FMT_MID: {
// 			space_lhs = ptr;
// 			space_lhs_len = (shot.full_len - shot.field_len) / 2;
// 			space_rhs = &ptr[space_lhs_len + shot.field_len];
// 			space_rhs_len = shot.full_len - space_lhs_len - shot.field_len;
// 		}; break;
//
// 		default: PANIC("invalid [align]");
// 	};
//
// 	u8 * field = &ptr[space_lhs_len];
// 	memset(space_lhs, style->fill, space_lhs_len);
//
// 	u64 i = 0;
// 	if (FMT_S_IS_QUOTES(style->opt)) {
// 		field[i] = '"';
// 		i += 1;
// 	};
//
// 	if (FMT_S_IS_ESCAPE(style->opt)) {
// 		fmt_str_escaped(field, src);
// 	} else {
// 		memcpy(field, src.raw, src.len);
// 	};
//
// 	i += shot.str_len;
//
// 	memset(space_rhs, style->fill, space_rhs_len);
//
// 	if (FMT_S_IS_QUOTES(style->opt)) {
// 		field[i] = '"';
// 		field += 1;
// 	};
//
// 	out->pos += shot.full_len;
// 	out->last = (StrMut){.raw = ptr, .len = shot.full_len};
//
// 	return true;
// };

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
// |> Number Formatting: f64                                                                        |
// |================================================================================================|

// |================================================================================================|
// |> Number Formatting: f64                                                                        |
// |================================================================================================|

FmtF64Shot fmt_f64_shot(f64 src, FmtNumStyle * style) {
	if (style->fill == 0) style->fill = ' ';

	union { f64 f; u64 u; } uval = { .f = src };
	u8 is_neg = (uval.u >> 63) & 1;
	u8 is_special = 0;

	if ((uval.u & 0x7FF0000000000000) == 0x7FF0000000000000) {
		is_special = ((uval.u & 0x000FFFFFFFFFFFFF) != 0) ? 1 : 2; // 1 => nan, 2 => inf
	};

	if (is_special) {
		u32 field_len = 3;
		if (is_neg || FMT_N_IS_SIGN(style->opt)) field_len += 1;
		return (FmtF64Shot){.field_len = field_len, .is_special = is_special, .is_neg = is_neg};
	};

	f64 abs_val = is_neg ? -src : src;
	if (abs_val > 18446744073709551615.0) abs_val = 18446744073709551615.0;

	u64 int_part = (u64)abs_val;
	f64 frac_part = abs_val - (f64)int_part;

	u8 prec = style->digits > 18 ? 18 : style->digits;
	f64 multiplier = 1.0;
	for (u8 i = 0; i < prec; ++i) multiplier *= 10.0;

	u64 frac_int = (u64)(frac_part * multiplier + 0.5);
	if (frac_int >= (u64)multiplier) { int_part += 1; frac_int = 0; };

	u64 temp_int = int_part;
	u8 int_len = 0;
	do { int_len += 1; temp_int /= 10; } while (temp_int > 0);

	u8 frac_len = prec;
	u64 temp_frac = frac_int;
	if (FMT_N_IS_DROP_ZEROS(style->opt) && frac_len > 0) {
		while (frac_len > 0 && (temp_frac % 10) == 0) { temp_frac /= 10; frac_len -= 1; };
	};

	u32 field_len = int_len;
	if (is_neg || FMT_N_IS_SIGN(style->opt)) field_len += 1;
	if (frac_len > 0) field_len += 1 + frac_len;

	return (FmtF64Shot){
		.field_len = field_len,
		.int_len = int_len, .frac_len = frac_len,
		.is_special = 0, .is_neg = is_neg,
	};
};

b8 fmt_f64_write(Fmt * out, f64 src, FmtNumStyle const * style, FmtF64Shot shot) {
	FmtLayout layout = fmt_layout(out, shot.field_len, style->width, FMT_GET_ALIGN(style->opt), style->fill);
	if (!layout.block) return false;

	u8 * w = layout.field;

	union { f64 f; u64 u; } uval = { .f = src };
	u8 is_neg = (uval.u >> 63) & 1;
	u8 is_special =
		((uval.u & 0x7FF0000000000000) == 0x7FF0000000000000) 
		? (((uval.u & 0x000FFFFFFFFFFFFF) != 0) ? 1 : 2) 
		: 0;

	if (FMT_N_IS_SIGN(style->opt)) {
		w[0] = is_neg ? '-' : '+'; w += 1;
	} else if (is_neg) {
		w[0] = '-'; w += 1;
	};

	if (is_special == 1) {
		w[0] = 'n'; w[1] = 'a'; w[2] = 'n'; w += 3;
	} else if (is_special == 2) {
		w[0] = 'i'; w[1] = 'n'; w[2] = 'f'; w += 3;
	} else {
		f64 abs_val = is_neg ? -src : src;
		if (abs_val > 18446744073709551615.0) abs_val = 18446744073709551615.0;

		u64 int_part = (u64)abs_val;
		f64 frac_part = abs_val - (f64)int_part;

		u8 prec = style->digits > 18 ? 18 : style->digits;
		f64 multiplier = 1.0;
		for (u8 i = 0; i < prec; ++i) multiplier *= 10.0;

		u64 frac_int = (u64)(frac_part * multiplier + 0.5);
		if (frac_int >= (u64)multiplier) { int_part += 1; frac_int = 0; };

		u8 ibuf[24];
		u8 iidx = sizeof(ibuf);
		u64 temp_int = int_part;
		do {
			iidx -= 1;
			ibuf[iidx] = '0' + (temp_int % 10);
			temp_int /= 10;
		} while (temp_int > 0);
		
		memcpy(w, &ibuf[iidx], shot.int_len);
		w += shot.int_len;

		if (shot.frac_len > 0) {
			w[0] = '.'; w += 1;

			u8 fbuf[24];
			u8 fidx = sizeof(fbuf);
			u64 temp_frac = frac_int;

			if (temp_frac > 0) do {
				fidx -= 1;
				fbuf[fidx] = '0' + (temp_frac % 10);
				temp_frac /= 10;
			} while (temp_frac > 0);

			u64 actual_frac_len = sizeof(fbuf) - fidx;
			u64 leading_zeros = shot.frac_len > actual_frac_len ? (shot.frac_len - actual_frac_len) : 0;
			for (u64 i = 0; i < leading_zeros; i++) { w[0] = '0'; w += 1; };
			if (actual_frac_len > 0) memcpy(w, &fbuf[fidx], actual_frac_len);
		};
	};

	out->pos += layout.full_len;
	out->last = (StrMut){.raw = layout.block, .len = layout.full_len};

	return true;
};

b8 fmt_f64_ex(Fmt * out, f64 src, FmtNumStyle * style) {
	return fmt_f64_write(out, src, style, fmt_f64_shot(src, style));
};

// |================================================================================================|
// |> Arguments Formatting                                                                          |
// |================================================================================================|

b8 fmt_mem_ex(Fmt * out, u64 src, FmtNumStyle * style) {
	f64 v; Str s; i8 prec = 1;
	if (src >= GB(4)) {
		v = (f64)src / GB(1); s = S("GiB");
		if (src % GB(1) == 0) prec = 0;
	} else if (src >= MB(4)) {
		v = (f64)src / MB(1); s = S("MiB");
		if (src % MB(1) == 0) prec = 0;
	} else if (src >= KB(4)) {
		v = (f64)src / KB(1); s = S("KiB");
		if (src % KB(1) == 0) prec = 0;
	} else {
		v = (f64)src; s = S("B"); prec = 0;
	};

	Fmt fmt = FMT_ON_STACK(128, out->flow);
	u32 width = style->width; style->width = 0;
	UNUSED(prec); // TODO: add f64
	fmt_u64_ex(&fmt, (u64)v, style);
	fmt_str_ex(&fmt, s, &(FmtStrStyle){0});

	FmtStrStyle final = (FmtStrStyle){.width = width, .fill = ' '};
	return fmt_str_ex(out, fmt_as_str(&fmt), &final);
};

b8 fmt_write(Fmt * fmt, FmtArg * args, u64 len) {
	u64 pos = fmt->pos;

	for (u64 i = 0; i < len; i += 1) {
		FmtArg arg = args[i];
		FmtMask flow = 1 << FMT_TAG_FLOW(arg.tag);
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
			case FMT_TY_F64: ok = fmt_f64_ex(
				fmt,
				arg.as.f64.src,
				&arg.as.f64.style
			); break;
			case FMT_TY_MEM: ok = fmt_mem_ex(
				fmt,
				arg.as.mem.src,
				&arg.as.mem.style
			); break;
			default: PANIC("invalid [arg.tag]");
		};

		if (!ok) return false;
	};

	fmt->last = STR_MUT(fmt->ptr + pos, fmt->pos - pos);
	return true;
};

