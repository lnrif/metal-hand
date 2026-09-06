#ifndef STD_FMT_DEF_H
#define STD_FMT_DEF_H

#include "std/core.h"
#include "std/mem/reg.h"
#include "std/str/str.h"

// |================================================================================================|
// |> Format State                                                                                  |
// |================================================================================================|

typedef struct {
	REG_EMBED(reg, ptr, any, raw, len);
	u64 pos; RegMan man;
	StrMut last;
} FmtOut;

// |================================================================================================|
// |> [FmtOut]: constructors                                                                        |

FmtOut fmt_init(RegMan man, RegInit init);
FmtOut fmt_from_slice(void * ptr, u64 len);
#define FMT_ON_STACK(size) fmt_from_slice(ALLOCA(size), size)

// |================================================================================================|
// |> [FmtOut]: mark and load                                                                       |

void fmt_reset(FmtOut * out);

// |================================================================================================|
// |> [FmtOut]: helpers                                                                             |

Str fmt_as_str(FmtOut const * out);

void fmt_rebase(FmtOut * out, Reg reg);
b8   fmt_reserve(FmtOut * out, u64 add);

// |================================================================================================|
// |> Raw Formatting                                                                                |
// |================================================================================================|

b8 fmt_raw_ex(FmtOut * out, Str lit);
#define fmt_raw(out, lit) fmt_raw_ex(out, S(lit))

// |================================================================================================|
// |> String Formatting                                                                             |
// |================================================================================================|

typedef enum: u8 {
	FMT_ALIGN_LEFT,
	FMT_ALING_CENTER,
	FMT_ALIGN_RIGHT,
} FmtAlign;

typedef struct {
	u32 width; u8 fill;
	b8 right;
	// FmtAlign align; u8 lt, rt;
	b8 escape; b8 quotes;
} FmtStrStyle;

typedef struct {
	u64 len;
	u64 str_len;
} FmtStrShot;

FmtStrShot fmt_str_shot(Str src, FmtStrStyle * style);
b8         fmt_str_write(FmtOut * out, Str src, FmtStrStyle const * style, FmtStrShot shot);
b8         fmt_str_ex(FmtOut * out, Str src, FmtStrStyle * style);

#define fmt_str(out, src, style...) fmt_str_ex(out, src, &(FmtStrStyle){style})
#define fmt_lit(out, src, style...) fmt_str(out, S(src), style)

// |================================================================================================|
// |> Number Formatting                                                                             |
// |================================================================================================|

typedef struct {
	u32 width; b8 right; u8 fill;
	u8 base; b8 sign; b8 prefix; u8 digits;
} FmtNumStyle;

typedef struct {
	u64 len;
	u64 field_len;
	u64 digits_len;
} FmtNumShot;

FmtNumShot fmt_num_shot(u64 src, b8 neg, FmtNumStyle * style);
b8         fmt_num_write(FmtOut * fmt, u64 src, b8 neg, FmtNumStyle const * style, FmtNumShot shot);

b8 fmt_num_ex(FmtOut * out, u64 src, u8 neg, FmtNumStyle * style);

// |================================================================================================|
// |> Number Formatting: u64                                                                        |
// |================================================================================================|

#define fmt_u64_shot(src, style...) fmt_num_shot(src, false, style)
#define fmt_u64(out, src, style...) fmt_num_ex(out, src, false, &(FmtNumStyle){style})

// |================================================================================================|
// |> Number Formatting: i64                                                                        |
// |================================================================================================|

b8 fmt_i64_ex(FmtOut * out, i64 src, FmtNumStyle * style);
#define fmt_i64(out, src, style...) fmt_i64_ex(out, src, &(FmtNumStyle){style})

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

#endif // !STD_FMT_DEF_H
