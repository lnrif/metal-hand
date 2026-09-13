#include "fmt.h"
#include "assert.h"
#include "core/types.h"
#include "mem/virt/virt.h"
#include "str/str.h"
#include "sys/sys.h"

#include <string.h>

////////////////////////////////
// len

u8 u64_len(u64 num, u64 base) {
	if (base < 2) return U8_MAX;
	u8 len = 1;
	while (num >= base) { num /= base; len += 1; };
	return len;
};

// static u8 i64_len(i64 num, u64 base) {
// 	if (num >= 0) return u64_len((u64)num, base);
// 	return u64_len(-(u64)num, base) + 1;
// };

static u32 number_len(u64 num, bool neg, FmtOpt opt) {
	u64 base; switch (FMT__NUM_BASE(opt.num)) {
		case FMT_NUM_BIN: base =  2; break;
		case FMT_NUM_DEC: base = 10; break;
		case FMT_NUM_HEX: base = 16; break;
		default: PANIC("");
	};

	u32 num_len = u64_len(num, base);
	if (neg || FMT__NUM_SIGN(opt.num)) num_len += 1;
	if (FMT__NUM_PREFIX(opt.num)) num_len += 2;

	if (FMT__NUM_FILL(opt.num)) return MAX(num_len, (u32)abs(opt.width));
	return num_len;
};

////////////////////////////////
// raw

static void fmt_u64_raw(
	u8 * ptr,
	usz len,
	u64 num,
	FmtNum opt
) {
	static char const * const BIN       = "01";
	static char const * const DEC       = "0123456789";
	static char const * const HEX_UPPER = "0123456789ABCDEF";
	static char const * const HEX_LOWER = "0123456789abcdef";

	char const * fmt; u64 b; u8 p;
	switch (FMT__NUM_BASE(opt)) {
		case FMT_NUM_BIN: fmt = BIN; b =  2; p = 'b';  break;
		case FMT_NUM_DEC: fmt = DEC; b = 10; p = '\0'; break;
		case FMT_NUM_HEX: {
			fmt = FMT__NUM_LOWER(opt) ? HEX_LOWER : HEX_UPPER;
			b = 16; p = 'x';
		} break;
		default: PANIC("invalid base");
	};

	usz i = len;
	for (;;) {
		ASSERT_DEBUG(i != 0, "not have memory for number");
		i -= 1; ptr[i] = (u8)fmt[num % b]; num /= b;
		if (num == 0) break;
	};

	if (FMT__NUM_FILL(opt)) {
		usz const low = FMT__NUM_PREFIX(opt) ? 2 : 0;
		for (;;) {
			if (i <= low) break;
			i -= 1;
			ptr[i] = '0';
		};
	};

	if (FMT__NUM_PREFIX(opt)) {
		ASSERT_DEBUG(i >= 2, "not have memory for prefix");
		ptr[i - 2] = '0';
		ptr[i - 1] = p;
	};
};

static u32 fmt_f64_raw(f64 value, i16 prec, u8 * buf, usz buf_len) {
	int len = (prec < 0)
		? snprintf((void*)buf, buf_len, "%.*g", -prec, value)
		: snprintf((void*)buf, buf_len, "%.*f", prec, value);
	if (len < 0) return 0;
	return (u32)len;
};

////////////////////////////////
// in buf

static u32 fmt_str_in_buf(u8 * ptr, usz len, Str str, FmtOpt opt, Str * dst) {
	if (str.ptr == NULL) return 0;
	if (ptr == NULL || str.len > U32_MAX) return U32_MAX;

	u32 const repeat   = opt.repeat == 0 ? 1 : opt.repeat;
	u32 const width    = (u32)abs(opt.width);
	u32 const one_len  = MAX(width, (u32)str.len);
	u32 const need_len = one_len * repeat;
	if (need_len > len) return need_len;

	u8 * at = ptr;
	if (width > str.len) {
		bool left  = opt.width > 0;
		u8 * space = left ? &ptr[str.len] : ptr;
		        at = left ? ptr : &ptr[width - str.len];
		memset(space, ' ', width - str.len);
	};

	memcpy(at, str.ptr, str.len);
	for (usz i = 1; i < repeat; i += 1) memcpy(&ptr[i * one_len], ptr, one_len);

	if (dst != NULL) *dst = (Str){.ptr = ptr, .len = need_len};
	return 0;
};

static u32 fmt_num_in_buf(u8 * ptr, usz len, u64 num, bool neg, FmtOpt opt, Str * dst) {
	if (ptr == NULL) return U32_MAX;

	u32 const repeat   = opt.repeat == 0 ? 1 : opt.repeat;
	u32 num_len        = number_len(num, neg, opt);
	u32 const width    = (u32)abs(opt.width);
	u32 const one_len  = MAX(width, num_len);
	u32 const need_len = one_len * repeat;
	if (need_len > len) return need_len;

	u8 * at = ptr;
	if (width > num_len) {
		bool left  = opt.width > 0;
		u8 * space = left ? &ptr[num_len] : ptr;
		        at = left ? ptr : &ptr[width - num_len];
		memset(space, ' ', width - num_len);
	};

	if (neg) {
		at[0] = '-'; at += 1; num_len -= 1;
	} else if (FMT__NUM_SIGN(opt.num)) {
		at[0] = num == 0 ? ' ' : '+'; at += 1; num_len -= 1;
	};

	fmt_u64_raw(at, num_len, num, opt.num);
	for (usz i = 1; i < repeat; i += 1) memcpy(&ptr[i * one_len], ptr, one_len);

	if (dst != NULL) *dst = (Str){.ptr = ptr, .len = need_len};
	return 0;
};

// static u32 fmt_u64_in_buf(u8 * ptr, usz len, u64 num, FmtOpt opt, Str * dst) {
// 	return fmt_num_in_buf(ptr, len, num, FALSE, opt, dst);
// };
//
// static u32 fmt_i64_in_buf(u8 * ptr, usz len, i64 num, FmtOpt opt, Str * dst) {
// 	u64 x = num < 0 ? -(u64)num : (u64)num;
// 	return fmt_num_in_buf(ptr, len, x, num < 0, opt, dst);
// };
//
// static u32 fmt_f64_in_buf(u8 * ptr, usz len, f64 value, FmtOpt opt, Str *dst) {
// 	u8 temp_buf[64];
// 	Str temp_str;
//
// 	temp_str.len = fmt_f64_raw(value, opt.prec, temp_buf, sizeof(temp_buf));
// 	temp_str.ptr = (u8 *)temp_str.len == 0 ? NULL : (u8 *)temp_buf;
//
// 	return fmt_str_in_buf(ptr, len, temp_str, opt, dst);
// };

static u32 fmt_mem_in_buf(u64 mem, u8 * ptr, usz len) {
	f64 v; Str s; i8 prec = 1;
	if (mem >= GB(1)) {
		v = (f64)mem / GB(1); s = STR("GiB");
		if (mem % GB(1) == 0) prec = 0;
	} else if (mem >= MB(1)) {
		v = (f64)mem / MB(1); s = STR("MiB");
		if (mem % MB(1) == 0) prec = 0;
	} else if (mem >= KB(1)) {
		v = (f64)mem / KB(1); s = STR("KiB");
		if (mem % KB(1) == 0) prec = 0;
	} else {
		v = (f64)mem; s = STR("B"); prec = 0;
	};

	u32 size = fmt_f64_raw(v, prec, ptr, len);
	if (size == 0) return 0;

	if (fmt_str_in_buf(&ptr[size], len - size, s, (FmtOpt){0}, NULL) != 0) return 0;
	return size + (u32)s.len;
};

////////////////////////////////
// bump

u32 fmt_bump_str(FmtBump * fmt, Str str, FmtOpt opt, Str * dst) {
	ASSERT_DEBUG(fmt != NULL, "expected [fmt]");
	usz const width = (usz)abs(opt.width);
	usz const len   = width > str.len ? width : str.len;

	u8 * ptr = bump_array(&fmt->bump, len, u8);
	return fmt_str_in_buf(ptr, len, str, opt, dst);
};

u32 fmt_bump_num(FmtBump * fmt, u64 num, bool neg, FmtOpt opt, Str * dst) {
	u32 const num_len  = number_len(num, neg, opt);
	u32 const width   = (u32)abs(opt.width);
	u32 const len     = width > num_len ? width : num_len;

	u8 * ptr = bump_array(&fmt->bump, len, u8);
	return fmt_num_in_buf(ptr, len, num, neg, opt, dst);
};

u32 fmt_bump_u64(FmtBump * fmt, u64 num, FmtOpt opt, Str * dst) {
	return fmt_bump_num(fmt, num, FALSE, opt, dst);
};

u32 fmt_bump_i64(FmtBump * fmt, i64 num, FmtOpt opt, Str * dst) {
	u64 x = num < 0 ? -(u64)num : (u64)num;
	return fmt_bump_num(fmt, x, num < 0, opt, dst);
};

u32 fmt_bump_f64(FmtBump * fmt, f64 num, FmtOpt opt, Str * dst) {
	u8 tmp[64]; Str str;
	str.len = fmt_f64_raw(num, opt.prec, tmp, sizeof(tmp));
	str.ptr = str.len == 0 ? NULL : (u8*)tmp;
	return fmt_bump_str(fmt, str, opt, dst);
};

u32 fmt_bump_mem(FmtBump * fmt, u64 mem, FmtOpt opt, Str * dst) {
	u8 tmp[64]; Str str; str.ptr = tmp;
	str.len = fmt_mem_in_buf(mem, tmp, LEN(tmp));
	if (str.len == 0) return U32_MAX;
	return fmt_bump_str(fmt, str, opt, dst);
};

Str fmt_bump_write_ex(FmtBump * fmt, FmtArg * ptr, usz len) {
	Str str = {.ptr = (void*)fmt->bump.pos, .len = 0};

	for (usz i = 0; i < len; i += 1) {
		FmtArg const arg = ptr[i];
		u32 result = 0;
		switch (arg.kind) {
			case FMT_ARG_STR:                   result = fmt_bump_str(fmt, arg.as.str, arg.opt, NULL);   break;
			case FMT_ARG_U64:                   result = fmt_bump_u64(fmt, arg.as.u64, arg.opt, NULL);   break;
			case FMT_ARG_I64:                   result = fmt_bump_i64(fmt, arg.as.i64, arg.opt, NULL);   break;
			case FMT_ARG_F64:                   result = fmt_bump_f64(fmt, arg.as.f64, arg.opt, NULL);   break;
			case FMT_ARG_MEM:                   result = fmt_bump_mem(fmt, arg.as.mem, arg.opt, NULL);   break;
			case FMT_ARG_COLOR: if (fmt->color) result = fmt_bump_str(fmt, arg.as.color, arg.opt, NULL); break;
			default: PANIC("invalid [arg.kind] (%hhu)", ptr[i].kind);
		};
		if (result != 0) return STR_NONE;
	};

	str.len = fmt->bump.pos - (uptr)str.ptr;
	return str;
};

Str fmt_bump_as_str(FmtBump const * fmt) {
	Bump bump = fmt->bump;
	return (Str){.ptr = (u8*)bump.beg, .len = bump.pos - bump.beg};
};

bool fmt_bump_flush(FmtBump * fmt) {
	ASSERT_DEBUG(fmt != NULL, "expected [fmt]");
	Bump const bump = fmt->bump;
	bool result = sys_write((void*)bump.beg, bump.pos - bump.beg);
	// reset, not cares what is [result]
	bump_reset(&fmt->bump);
	return result;
};

void fmt_bump_reset(FmtBump * fmt) {
	bump_reset(&fmt->bump);
};

////////////////////////////////
// virt

u32 fmt_virt_str(FmtVirt * fmt, Str str, FmtOpt opt, Str * dst) {
	ASSERT_DEBUG(fmt != NULL, "expected [fmt]");
	u32 const repeat   = opt.repeat == 0 ? 1 : opt.repeat;
	u32 const width    = (u32)abs(opt.width);
	u32 const one_len  = MAX(width, (u32)str.len);
	u32 const need_len = one_len * repeat;
	u8 * ptr = virt_bump_array(&fmt->virt, need_len, u8);
	if (ptr == NULL || !virt_bump_commit(&fmt->virt)) return U32_MAX;
	return fmt_str_in_buf(ptr, need_len, str, opt, dst);
};

u32 fmt_virt_num(FmtVirt * fmt, u64 num, bool neg, FmtOpt opt, Str * dst) {
	u32 const num_len = number_len(num, neg, opt);
	u32 const width   = (u32)abs(opt.width);
	u32 const len     = width > num_len ? width : num_len;
	u8 * ptr = virt_bump_array(&fmt->virt, len, u8);
	if (ptr == NULL || !virt_bump_commit(&fmt->virt)) return U32_MAX;
	return fmt_num_in_buf(ptr, len, num, neg, opt, dst);
};

u32 fmt_virt_u64(FmtVirt * fmt, u64 num, FmtOpt opt, Str * dst) {
	return fmt_virt_num(fmt, num, FALSE, opt, dst);
};

u32 fmt_virt_i64(FmtVirt * fmt, i64 num, FmtOpt opt, Str * dst) {
	u64 x = num < 0 ? -(u64)num : (u64)num;
	return fmt_virt_num(fmt, x, num < 0, opt, dst);
};

u32 fmt_virt_f64(FmtVirt * fmt, f64 num, FmtOpt opt, Str * dst) {
	u8 tmp[64]; Str str;
	str.len = fmt_f64_raw(num, opt.prec, tmp, sizeof(tmp));
	str.ptr = str.len == 0 ? NULL : (u8*)tmp;
	return fmt_virt_str(fmt, str, opt, dst);
};

u32 fmt_virt_mem(FmtVirt * fmt, u64 mem, FmtOpt opt, Str * dst) {
	u8 tmp[64]; Str str; str.ptr = tmp;
	str.len = fmt_mem_in_buf(mem, tmp, LEN(tmp));
	if (str.len == 0) return U32_MAX;
	return fmt_virt_str(fmt, str, opt, dst);
};

Str fmt_virt_as_str(FmtVirt const * fmt) {
	Bump bump = fmt->virt.bump;
	return (Str){.ptr = (u8*)bump.beg, .len = bump.pos - bump.beg};
};

Str fmt_virt_write_ex(FmtVirt * fmt, FmtArg * ptr, usz len) {
	Str str = {.ptr = (void*)fmt->virt.bump.pos, .len = 0};

	for (usz i = 0; i < len; i += 1) {
		FmtArg const arg = ptr[i];
		u32 result = 0;
		switch (arg.kind) {
			case FMT_ARG_STR:                   result = fmt_virt_str(fmt, arg.as.str, arg.opt, NULL);   break;
			case FMT_ARG_U64:                   result = fmt_virt_u64(fmt, arg.as.u64, arg.opt, NULL);   break;
			case FMT_ARG_I64:                   result = fmt_virt_i64(fmt, arg.as.i64, arg.opt, NULL);   break;
			case FMT_ARG_F64:                   result = fmt_virt_f64(fmt, arg.as.f64, arg.opt, NULL);   break;
			case FMT_ARG_MEM:                   result = fmt_virt_mem(fmt, arg.as.mem, arg.opt, NULL);   break;
			case FMT_ARG_COLOR: if (fmt->color) result = fmt_virt_str(fmt, arg.as.color, arg.opt, NULL); break;
			default: PANIC("invalid [arg.kind] (%hhu)", ptr[i].kind);
		};
		if (result != 0) return STR_NONE;
	};

	str.len = fmt->virt.bump.pos - (uptr)str.ptr;
	return str;
};

bool fmt_virt_flush(FmtVirt * fmt) {
	ASSERT_DEBUG(fmt != NULL, "expected [fmt]");
	Bump const bump = fmt->virt.bump;
	bool result = sys_write((void*)bump.beg, bump.pos - bump.beg);
	// reset, not cares what is [result]
	virt_bump_reset(&fmt->virt);
	return result;
};

void fmt_virt_reset(FmtVirt * fmt) {
	virt_bump_reset(&fmt->virt);
};

