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

typedef struct {
	REG_EMBED(reg, ptr, any, raw, len);
	u64 pos; RegMan man;
	StrMut last;
} Fmt;

// |================================================================================================|
// |> [Fmt]: constructors                                                                           |

Fmt fmt_init(RegMan man);
Fmt fmt_from_slice(void * ptr, u64 len);
#define FMT_ON_STACK(size) fmt_from_slice(ALLOCA(size), size)

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
b8         fmt_str_write(Fmt * out, Str src, FmtStrStyle const * style, FmtStrShot shot);
b8         fmt_str_ex(Fmt * out, Str src, FmtStrStyle * style);

#define fmt_str(out, src, style...) fmt_str_ex(out, src, &(FmtStrStyle){style})

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
b8         fmt_num_write(Fmt * fmt, u64 src, b8 neg, FmtNumStyle const * style, FmtNumShot shot);

b8 fmt_num_ex(Fmt * out, u64 src, u8 neg, FmtNumStyle * style);

// |================================================================================================|
// |> Number Formatting: u64                                                                        |
// |================================================================================================|

#define fmt_u64_shot(src, style...) fmt_num_shot(src, false, style)
#define fmt_u64(out, src, style...) fmt_num_ex(out, src, false, &(FmtNumStyle){style})

// |================================================================================================|
// |> Number Formatting: i64                                                                        |
// |================================================================================================|

b8 fmt_i64_ex(Fmt * out, i64 src, FmtNumStyle * style);
#define fmt_i64(out, src, style...) fmt_i64_ex(out, src, &(FmtNumStyle){style})

// |================================================================================================|
// |> ANSI colors                                                                                   |

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
