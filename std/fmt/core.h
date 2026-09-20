#ifndef STD_FMT_CORE_H
#define STD_FMT_CORE_H

#include "std/core.h"
#include "std/mem/reg.h"
#include "std/str/core.h"
#include "std/stream/core.h"
// #include "std/fs/core.h"

// |================================================================================================|
// |> Format State                                                                                  |
// |================================================================================================|

typedef u32 FmtMask;

#define FMT_SET_TEXT  ((u32)0b01)
#define FMT_SET_COLOR ((u32)0b10)

#define FMT_FLOW_TEXT  ((u8)0)
#define FMT_FLOW_COLOR ((u8)1)

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
	FMT_TY_F64 = 0b00000011,
	FMT_TY_MEM = 0b00000100,
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

// [str_len] <= [field_len] <= [full_len]
typedef struct {
	u64 full_len;
	u64 field_len;
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
	FMT_N_SIGN       = 0b00010000,
	// enable: 0b1111, 0o7777, 0xFFFF (default)
	FMT_N_PREFIX     = 0b00000000,
	// disable:  1111,   7777,   FFFF
	FMT_N_NO_PREFIX  = 0b00100000,
	// [0b]1010_1010, 1_000_000, [0x]FFFF_FFFF
	FMT_N_PRETTY     = 0b01000000,
	// ...
	FMT_N_DROP_ZEROS = 0b10000000,
} FmtNumOpt;

#define FMT_N_GET_BASE(opt)      ((opt) & FMT_N_BASE)
#define FMT_N_IS_PREFIX(opt)     (((opt) & FMT_N_NO_PREFIX) == FMT_N_PREFIX)
#define FMT_N_IS_SIGN(opt)       (((opt) & FMT_N_SIGN)   == FMT_N_SIGN)
#define FMT_N_IS_PRETTY(opt)     (((opt) & FMT_N_PRETTY) == FMT_N_PRETTY)
#define FMT_N_IS_DROP_ZEROS(opt) (((opt) & FMT_N_DROP_ZEROS) == FMT_N_DROP_ZEROS)

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
// |> Number Formatting: f64                                                                        |
// |================================================================================================|

typedef struct {
	u32 field_len;
	u8 int_len; u8 frac_len;
	// 0 = normal, 1 = nan, 2 = inf
	u8 is_special; u8 is_neg;
} FmtF64Shot;

b8 fmt_f64_ex(Fmt * out, f64 src, FmtNumStyle * style);

// |================================================================================================|
// |> Arguments Formatting                                                                          |
// |================================================================================================|

typedef union {
	union {
		struct { Str src; u8 _pad[0]; FmtStrStyle style; } str;
		struct { u64 src; u8 _pad[8]; FmtNumStyle style; } u64;
		struct { i64 src; u8 _pad[8]; FmtNumStyle style; } i64;
		struct { f64 src; u8 _pad[8]; FmtNumStyle style; } f64;
		struct { u64 src; u8 _pad[8]; FmtNumStyle style; } mem;
	} as;
	struct {
		u8 _pad[23];
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

static inline FmtArg fmt_arg_f64(f64 src, FmtArgNumStyle style) {
	return (FmtArg){.as.f64 = {.src = src, .style = fmt_num_style(FMT_TY_F64, style)}};
};

static inline FmtArg fmt_arg_mem(u64 src, FmtArgNumStyle style) {
	return (FmtArg){.as.mem = {.src = src, .style = fmt_num_style(FMT_TY_MEM, style)}};
};

#define FMT_STR(str, style...) fmt_arg_str(str, (FmtArgStrStyle){style})
#define FMT_U64(u64, style...) fmt_arg_u64(u64, (FmtArgNumStyle){style})
#define FMT_I64(i64, style...) fmt_arg_i64(i64, (FmtArgNumStyle){style})
#define FMT_F64(f64, style...) fmt_arg_f64(f64, (FmtArgNumStyle){style})
#define FMT_MEM(mem, style...) fmt_arg_mem(mem, (FmtArgNumStyle){style})

#define FMT_LIT(lit, style...) FMT_STR(S(lit), style)
#define FMT_REPEAT(lit, count) FMT_LIT("", .fill = (lit), .width = (count))

#define FMT_COLOR(str) FMT_STR(str, .flow = FMT_FLOW_COLOR)

b8 fmt_write(Fmt * fmt, FmtArg * args, u64 len);

#define FMT_EX(fmt, args...) fmt_write(fmt, args, LEN(args))
#define FMT(fmt, args...) FMT_EX(fmt, (FmtArg[]){args})

// |================================================================================================|
// |> ANSI                                                                                          |
// |================================================================================================|

#define ANSI_BOLD         "\x1B[1m"
#define ANSI_FAINT        "\x1B[2m"
#define ANSI_ITALIC       "\x1B[3m"
#define ANSI_UNDERLINE    "\x1B[4m"
#define ANSI_BLINK        "\x1B[5m"
#define ANSI_REV          "\x1B[7m"
#define ANSI_INVISIBLE    "\x1B[8m"

#define ANSI_BOLD_RESET   "\x1B[22m"
#define ANSI_ITALIC_RESET "\x1B[23m"
#define ANSI_UNDER_RESET  "\x1B[24m"
#define ANSI_BLINK_RESET  "\x1B[25m"
#define ANSI_REV_RESET    "\x1B[27m"
#define ANSI_RESET        "\x1B[m"

#define ANSI_FG_RESET "\x1B[39m"
#define ANSI_BG_RESET "\x1B[49m"
#define ANSI_RESET    "\x1B[m"

#define ANSI_BLACK        "\x1B[30m"
#define ANSI_RED          "\x1B[31m"
#define ANSI_GREEN        "\x1B[32m"
#define ANSI_YELLOW       "\x1B[33m"
#define ANSI_BLUE         "\x1B[34m"
#define ANSI_MAGENTA      "\x1B[35m"
#define ANSI_CYAN         "\x1B[36m"
#define ANSI_WHITE        "\x1B[37m"

#define ANSI_BRIGHT_BLACK   "\x1B[90m"
#define ANSI_BRIGHT_RED     "\x1B[91m"
#define ANSI_BRIGHT_GREEN   "\x1B[92m"
#define ANSI_BRIGHT_YELLOW  "\x1B[93m"
#define ANSI_BRIGHT_BLUE    "\x1B[94m"
#define ANSI_BRIGHT_MAGENTA "\x1B[95m"
#define ANSI_BRIGHT_CYAN    "\x1B[96m"
#define ANSI_BRIGHT_WHITE   "\x1B[97m"

#define ANSI_BG_BLACK     "\x1B[40m"
#define ANSI_BG_RED       "\x1B[41m"
#define ANSI_BG_GREEN     "\x1B[42m"
#define ANSI_BG_YELLOW    "\x1B[43m"
#define ANSI_BG_BLUE      "\x1B[44m"
#define ANSI_BG_MAGENTA   "\x1B[45m"
#define ANSI_BG_CYAN      "\x1B[46m"
#define ANSI_BG_WHITE     "\x1B[47m"

#define ANSI_BG_BRIGHT_BLACK   "\x1B[100m"
#define ANSI_BG_BRIGHT_RED     "\x1B[101m"
#define ANSI_BG_BRIGHT_GREEN   "\x1B[102m"
#define ANSI_BG_BRIGHT_YELLOW  "\x1B[103m"
#define ANSI_BG_BRIGHT_BLUE    "\x1B[104m"
#define ANSI_BG_BRIGHT_MAGENTA "\x1B[105m"
#define ANSI_BG_BRIGHT_CYAN    "\x1B[106m"
#define ANSI_BG_BRIGHT_WHITE   "\x1B[107m"

// |================================================================================================|
// |> [Fmt]: ANSI                                                                                   |
// |================================================================================================|

#define FMT_BOLD         FMT_COLOR(S(ANSI_BOLD))
#define FMT_FAINT        FMT_COLOR(S(ANSI_FAINT))
#define FMT_ITALIC       FMT_COLOR(S(ANSI_ITALIC))
#define FMT_UNDERLINE    FMT_COLOR(S(ANSI_UNDERLINE))
#define FMT_BLINK        FMT_COLOR(S(ANSI_BLINK))
#define FMT_REVERSE      FMT_COLOR(S(ANSI_REVERSE))
#define FMT_INVISIBLE    FMT_COLOR(S(ANSI_INVISIBLE))

#define FMT_BOLD_RESET   FMT_COLOR(S(ANSI_BOLD_RESET))
#define FMT_ITALIC_RESET FMT_COLOR(S(ANSI_ITALIC_RESET))
#define FMT_UNDER_RESET  FMT_COLOR(S(ANSI_UNDER_RESET))
#define FMT_BLINK_RESET  FMT_COLOR(S(ANSI_BLINK_RESET))
#define FMT_REV_RESET    FMT_COLOR(S(ANSI_REV_RESET))
#define FMT_RESET        FMT_COLOR(S(ANSI_RESET))

#define FMT_FG_RESET FMT_COLOR(S(ANSI_FG_RESET))
#define FMT_BG_RESET FMT_COLOR(S(ANSI_BG_RESET))
#define FMT_RESET    FMT_COLOR(S(ANSI_RESET))

#define FMT_BLACK        FMT_COLOR(S(ANSI_BLACK))
#define FMT_RED          FMT_COLOR(S(ANSI_RED))
#define FMT_GREEN        FMT_COLOR(S(ANSI_GREEN))
#define FMT_YELLOW       FMT_COLOR(S(ANSI_YELLOW))
#define FMT_BLUE         FMT_COLOR(S(ANSI_BLUE))
#define FMT_MAGENTA      FMT_COLOR(S(ANSI_MAGENTA))
#define FMT_CYAN         FMT_COLOR(S(ANSI_CYAN))
#define FMT_WHITE        FMT_COLOR(S(ANSI_WHITE))

#define FMT_BRIGHT_BLACK   FMT_COLOR(S(ANSI_BRIGHT_BLACK))
#define FMT_BRIGHT_RED     FMT_COLOR(S(ANSI_BRIGHT_RED))
#define FMT_BRIGHT_GREEN   FMT_COLOR(S(ANSI_BRIGHT_GREEN))
#define FMT_BRIGHT_YELLOW  FMT_COLOR(S(ANSI_BRIGHT_YELLOW))
#define FMT_BRIGHT_BLUE    FMT_COLOR(S(ANSI_BRIGHT_BLUE))
#define FMT_BRIGHT_MAGENTA FMT_COLOR(S(ANSI_BRIGHT_MAGENTA))
#define FMT_BRIGHT_CYAN    FMT_COLOR(S(ANSI_BRIGHT_CYAN))
#define FMT_BRIGHT_WHITE   FMT_COLOR(S(ANSI_BRIGHT_WHITE))

#define FMT_BG_BLACK     FMT_COLOR(S(ANSI_BG_BLACK))
#define FMT_BG_RED       FMT_COLOR(S(ANSI_BG_RED))
#define FMT_BG_GREEN     FMT_COLOR(S(ANSI_BG_GREEN))
#define FMT_BG_YELLOW    FMT_COLOR(S(ANSI_BG_YELLOW))
#define FMT_BG_BLUE      FMT_COLOR(S(ANSI_BG_BLUE))
#define FMT_BG_MAGENTA   FMT_COLOR(S(ANSI_BG_MAGENTA))
#define FMT_BG_CYAN      FMT_COLOR(S(ANSI_BG_CYAN))
#define FMT_BG_WHITE     FMT_COLOR(S(ANSI_BG_WHITE))

#define FMT_BG_BRIGHT_BLACK   FMT_COLOR(S(ANSI_BG_BRIGHT_BLACK))
#define FMT_BG_BRIGHT_RED     FMT_COLOR(S(ANSI_BG_BRIGHT_RED))
#define FMT_BG_BRIGHT_GREEN   FMT_COLOR(S(ANSI_BG_BRIGHT_GREEN))
#define FMT_BG_BRIGHT_YELLOW  FMT_COLOR(S(ANSI_BG_BRIGHT_YELLOW))
#define FMT_BG_BRIGHT_BLUE    FMT_COLOR(S(ANSI_BG_BRIGHT_BLUE))
#define FMT_BG_BRIGHT_MAGENTA FMT_COLOR(S(ANSI_BG_BRIGHT_MAGENTA))
#define FMT_BG_BRIGHT_CYAN    FMT_COLOR(S(ANSI_BG_BRIGHT_CYAN))
#define FMT_BG_BRIGHT_WHITE   FMT_COLOR(S(ANSI_BG_BRIGHT_WHITE))

#define FMT_GREY   FMT_BRIGHT_BLACK
#define FMT_ORANGE FMT_COLOR(S("\x1B[38;5;208m"))

#define FMT_LOC(flow) \
	FMT_CYAN, \
	FMT_LIT("--> "), \
	FMT_STR(str_z_init(flow.get).str), \
	FMT_LIT("\n")

#endif // !STD_FMT_CORE_H
