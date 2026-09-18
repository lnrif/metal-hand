#ifndef STD_FMT_CORE_H
#define STD_FMT_CORE_H

#include "std/core.h"
#include "std/mem/reg.h"
#include "std/str/core.h"
#include "std/stream/core.h"
#include "std/fs/core.h"

// |================================================================================================|
// |> Format State                                                                                  |
// |================================================================================================|

typedef u32 FmtMask;

typedef struct {
	REG_EMBED(reg, ptr, any, raw, len);
	u64 pos; RegMan man;
	StrMut last;
	FmtMask flow;
} Fmt;

// |================================================================================================|
// |> [Fmt]: constructors                                                                           |

Fmt fmt_init(RegMan man, FmtMask flow);
Fmt fmt_from_slice(void * ptr, u64 len, FmtMask flow);
#define FMT_ON_STACK(size, mask) fmt_from_slice(ALLOCA(size), size, mask)

// |================================================================================================|
// |> [Fmt]: mark and load                                                                          |

void fmt_reset(Fmt * out);

// |================================================================================================|
// |> [Fmt]: helpers                                                                                |

Str fmt_as_str(Fmt const * out);
b8 fmt_flush_stream(Fmt * fmt, Stream * stream);
b8 fmt_flush_file(Fmt * fmt, u8z const * path);

void fmt_rebase(Fmt * out, Reg reg);
b8   fmt_reserve(Fmt * out, u64 add);

// |================================================================================================|
// |> Raw Formatting                                                                                |
// |================================================================================================|

b8 fmt_lit_ex(Fmt * out, Str bs);
#define fmt_lit(out, bs) fmt_lit_ex(out, S(bs))
#define fmt_lit_z(out, bs) fmt_lit_ex(out, S(bs "\0"))

// |================================================================================================|
// |> String Formatting                                                                             |
// |================================================================================================|

// u4:u4
typedef enum: u8 {
	FMT_TY_STR = 0b00000000,
	FMT_TY_U64 = 0b00000001,
	FMT_TY_I64 = 0b00000010,
	FMT_TY_MEM = 0b00000011,
} FmtTy;

typedef u8 FmtTag;

#define FMT_TAG(ty, flow) ((FmtTag)((flow << 4) | (ty)))
#define FMT_TAG_TY(tag)   ((tag) & 0b1111)
#define FMT_TAG_FLOW(tag) ((tag) >> 4)

enum: u8 {
	FMT_LHS   = 0b00000000,
	FMT_MID   = 0b00000001,
	FMT_RHS   = 0b00000010,
	FMT_ALIGN = 0b00000011,
};

#define FMT_GET_ALIGN(opt) ((opt) & FMT_ALIGN)
#define FMT_IS_LHS(opt) (FMT_GET_ALIGN(opt) == FMT_LHS)
#define FMT_IS_MID(opt) (FMT_GET_ALIGN(opt) == FMT_MID)
#define FMT_IS_RHS(opt) (FMT_GET_ALIGN(opt) == FMT_RHS)

typedef enum: u8 {
	FMT_S_LHS    = FMT_LHS,
	FMT_S_MID    = FMT_MID,
	FMT_S_RHS    = FMT_RHS,
	FMT_S_QUOTES = 0b00000100,
	FMT_S_ESCAPE = 0b00001000,
} FmtStrOpt;

#define FMT_S_IS_QUOTES(opt) (((opt) & FMT_S_QUOTES) == FMT_S_QUOTES)
#define FMT_S_IS_ESCAPE(opt) (((opt) & FMT_S_ESCAPE) == FMT_S_ESCAPE)

typedef struct {
	// minimal width
	u32 width;
	// fill ascii char, by default 0 => ' ' (<space>)
	u8 fill;
	// by default 0
	u8 flow;
	// by default 0 (check [FmtStrOpt])
	FmtStrOpt opt;
} FmtArgStrStyle;

STATIC_ASSERT(sizeof(FmtArgStrStyle) == 8);

typedef struct {
	u64 full_len;
	u64 str_len;
} FmtStrShot;

typedef struct {
	// minimal width
	u32 width;
	// fill ascii char, by default 0 => ' ' (<space>)
	u8 fill;
	FmtStrOpt opt;
	u8 _pad[1];
	FmtTag tag;
} FmtStrStyle; // AFTER CHANGE CHECK [FmtArg.(tag, flow)]

FmtStrShot fmt_str_shot(Str src, FmtStrStyle * style);
b8         fmt_str_write(Fmt * out, Str src, FmtStrStyle const * style, FmtStrShot shot);
b8         fmt_str_ex(Fmt * out, Str src, FmtStrStyle * style);

#define fmt_str(out, src, style...) fmt_str_ex(out, src, &(FmtStrStyle){style})

// |================================================================================================|
// |> Number Formatting                                                                             |
// |================================================================================================|

typedef enum: u8 {
	FMT_N_LHS = FMT_LHS,
	FMT_N_MID = FMT_MID,
	FMT_N_RHS = FMT_RHS,

	FMT_N_DEC = 0b00000000,
	FMT_N_BIN = 0b00000100,
	FMT_N_OCT = 0b00001000,
	FMT_N_HEX = 0b00001100, FMT_N_BASE = FMT_N_HEX,

	// +1, +0, -0, -1
	FMT_N_SIGN      = 0b00100000,
	// enable: 0b1111, 0o7777, 0xFFFF (default)
	FMT_N_PREFIX    = 0b00000000,
	// disable:  1111,   7777,   FFFF
	FMT_N_NO_PREFIX = 0b01000000,
	// [0b]1010_1010, 1_000_000, [0x]FFFF_FFFF
	FMT_N_PRETTY    = 0b10000000,
} FmtNumOpt;

#define FMT_N_GET_BASE(opt)  ((opt) & FMT_N_BASE)
#define FMT_N_IS_PREFIX(opt) (((opt) & FMT_N_NO_PREFIX) == FMT_N_PREFIX)
#define FMT_N_IS_SIGN(opt)   (((opt) & FMT_N_SIGN)   == FMT_N_SIGN)
#define FMT_N_IS_PRETTY(opt) (((opt) & FMT_N_PRETTY) == FMT_N_PRETTY)

typedef struct {
	// minimal width
	u32 width;
	// fill ascii char, by default 0 => ' ' (<space>)
	u8 fill;
	// minimal digit count (fill with '0')
	u8 digits;
	// by default 0
	u8 flow;
	// by default 0 (check [FmtNumOpt])
	FmtNumOpt opt;
} FmtArgNumStyle;

typedef struct {
	// minimal width
	u32 width;
	// fill ascii char, by default 0 => ' ' (<space>)
	u8 fill;
	// minimal digit count (fill with '0')
	u8 digits;
	FmtNumOpt opt;
	FmtTag tag;
} FmtNumStyle; // AFTER CHANGE CHECK [FmtArg.(tag, flow)]

typedef struct {
	u64 full_len;
	u64 field_len;
	u64 digits_len;
} FmtNumShot;

FmtNumShot fmt_num_shot(u64 src, b8 neg, FmtNumStyle * style);
b8         fmt_num_write(Fmt * fmt, u64 src, b8 neg, FmtNumStyle const * style, FmtNumShot shot);

b8 fmt_num_ex(Fmt * out, u64 src, u8 neg, FmtNumStyle * style);

// |================================================================================================|
// |> Number Formatting: u64                                                                        |
// |================================================================================================|

#define fmt_u64_shot(src, style...) fmt_num_shot(src, false, style)
#define fmt_u64_ex(out, src, style...) fmt_num_ex(out, src, false, style)
#define fmt_u64(out, src, style...) fmt_u64_ex(out, src, &(FmtNumStyle){style})

// |================================================================================================|
// |> Number Formatting: i64                                                                        |
// |================================================================================================|

b8 fmt_i64_ex(Fmt * out, i64 src, FmtNumStyle * style);
#define fmt_i64(out, src, style...) fmt_i64_ex(out, src, &(FmtNumStyle){style})

// |================================================================================================|
// |> Arguments Formatting                                                                          |
// |================================================================================================|

typedef union {
	union {
		struct { Str src; FmtStrStyle style; } str;
		struct { u64 src; FmtNumStyle style; } u64;
		struct { i64 src; FmtNumStyle style; } i64;
		struct { u64 src; FmtNumStyle style; } mem;
	} as;
	struct {
		u8 _pad[10 + 6 + 7]; // 67
		FmtTag tag;
	};
} FmtArg;

STATIC_ASSERT(sizeof(FmtArg) == 24);

static inline FmtStrStyle fmt_str_style(FmtTy ty, FmtArgStrStyle style) {
	return (FmtStrStyle){
		.width = style.width,
		.fill = style.fill,
		.opt = style.opt,
		.tag = FMT_TAG(ty, style.flow),
	};
};

static inline FmtNumStyle fmt_num_style(FmtTy ty, FmtArgNumStyle style) {
	return (FmtNumStyle){
		.width = style.width,
		.fill = style.fill,
		.digits = style.digits,
		.opt = style.opt,
		.tag = FMT_TAG(ty, style.flow),
	};
};

static inline FmtArg fmt_arg_str(Str src, FmtArgStrStyle style) {
	return (FmtArg){.as.str = {.src = src, .style = fmt_str_style(FMT_TY_STR, style)}};
};

static inline FmtArg fmt_arg_u64(u64 src, FmtArgNumStyle style) {
	return (FmtArg){.as.u64 = {.src = src, .style = fmt_num_style(FMT_TY_U64, style)}};
};

static inline FmtArg fmt_arg_i64(i64 src, FmtArgNumStyle style) {
	return (FmtArg){.as.i64 = {.src = src, .style = fmt_num_style(FMT_TY_I64, style)}};
};

static inline FmtArg fmt_arg_mem(u64 src, FmtArgNumStyle style) {
	return (FmtArg){.as.mem = {.src = src, .style = fmt_num_style(FMT_TY_MEM, style)}};
};

#define FMT_STR(str, style...) fmt_arg_str(str, (FmtArgStrStyle){style})
#define FMT_U64(u64, style...) fmt_arg_u64(u64, (FmtArgMumStyle){style})
#define FMT_I64(i64, style...) fmt_arg_i64(i64, (FmtArgMumStyle){style})
#define FMT_MEM(mem, style...) fmt_arg_mem(mem, (FmtArgMumStyle){style})

#define FMT_LIT(lit, style...) FMT_STR(S(lit), style)

b8 fmt_write(Fmt * fmt, FmtArg * args, u64 len);

#define FMT_EX(fmt, args...) fmt_write(fmt, args, LEN(args))
#define FMT(fmt, args...) FMT_EX(fmt, (FmtArg[]){args})

// |================================================================================================|
// |> ANSI colors                                                                                   |
// |================================================================================================|

#define ANSI_BOLD       "\x1B[1m"
#define ANSI_BOLD_RESET "\x1B[22m"

#define ANSI_BLACK   "\x1B[30m"
#define ANSI_RED     "\x1B[31m"
#define ANSI_GREEN   "\x1B[32m"
#define ANSI_YELLOW  "\x1B[33m"
#define ANSI_BLUE    "\x1B[34m"
#define ANSI_MAGENTA "\x1B[35m"
#define ANSI_CYAN    "\x1B[36m"
#define ANSI_WHITE   "\x1B[37m"

#define ANSI_8B(n)        "\x1B[38;5;" n "m"
#define ANSI_24B(r, g, b) "\x1B[38;2;" r ";" g ";" b "m"

#define ANSI_FG_RESET   "\x1B[39m"
#define ANSI_RESET      "\x1B[m" // "\x1B[0m"

// |================================================================================================|
// |> TRASH                                                                                         |

// StrMut bump_raw_lit_ex(BumpRaw * bump, Str str);
// #define bump_raw_lit(bump, lit) bump_raw_lit_ex(bump, S(lit))
//
// StrMut bump_gen_lit_ex(Bump * bump, Str str);
// #define bump_gen_lit(bump, lit) bump_lit_ex(bump, S(lit))
//
// StrMut bump_raw_str_ex(BumpRaw * bump, Str str, FmtStr fmt);
// #define bump_raw_str(bump, str, fmt...) bump_raw_str_ex(bump, str, (FmtStr){fmt})
//
// StrMut bump_raw_num_ex(BumpRaw * bump, b8 neg, u64 num, FmtNum fmt);
// #define bump_raw_num(bump, neg, num, fmt...) bump_raw_num_ex(bump, neg, num, (FmtNum){fmt})
//
// #define bump_raw_u64_ex(bump, num, fmt...) bump_raw_num_ex(bump, false, num, fmt)
// #define bump_raw_u64(bump, num, fmt...) bump_raw_u64_ex(bump, false, num, (FmtNum){fmt})
//
// StrMut bump_raw_i64_ex(BumpRaw * bump, i64 num, FmtNum fmt);
// #define bump_raw_i64(bump, num, fmt...) bump_raw_i64_ex(bump, num, (FmtNum){fmt})
//
// StrMut bump_str_ex(Bump * bump, Str str, FmtStr fmt);
// #define bump_str(bump, str, fmt...)      bump_str_ex(bump, str, (FmtStr){fmt})
//
// StrMut bump_num_ex(Bump * bump, u64 num, b8 neg, FmtNum fmt);
// #define bump_num(bump, num, neg, fmt...) bump_num_ex(bump, num, neg, (FmtNum){fmt})
//
// #define bump_u64_ex(bump, num, fmt...)   bump_num_ex(bump, num, false, fmt)
// #define bump_u64(bump, num, fmt...)      bump_u64_ex(bump, num, false, (FmtNum){fmt})
//
// StrMut bump_i64_ex(Bump * bump, i64 num, FmtNum fmt);
// #define bump_i64(bump, num, fmt...) bump_i64_ex(bump, num, (FmtNum){fmt})

#endif // !STD_FMT_CORE_H
