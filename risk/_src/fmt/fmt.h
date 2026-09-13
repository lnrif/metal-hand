#ifndef RK_FMT_H
#define RK_FMT_H

#include "str/str.h"
#include "mem/virt/virt.h"

u8 u64_len(u64 num, u64 base);

ENUM(FmtNum, u8) {
	// 0..=1
	FMT_NUM_DEC       = 0b00000000, // 0..=10
	FMT_NUM_BIN       = 0b00000001, // 0..=1
	FMT_NUM_HEX       = 0b00000010, // 0..=F (15)
	FMT_NUM_LOWER     = 0b00000100, // 0xDEAD -> 0xdead
	FMT_NUM_NO_PREFIX = 0b00001000, // 0xDEAD -> DEAD

	// 10 -> +10
	//  0 ->   0 <- with space
	// -1 ->  -1
	FMT_NUM_SHOW_SIGN = 0b00010000,
	FMT_NUM_FILL      = 0b00100000,
};

#define FMT__NUM_BASE(num)     (((num) >> 0) & 0b11)
#define FMT__NUM_LOWER(num)    (((num) >> 2) & 0b01)
#define FMT__NUM_SIGN(num)     (((num) >> 4) & 0b01)
#define FMT__NUM_FILL(num)     (((num) >> 5) & 0b01)

#define FMT__NUM_PREFIX_BIT(num) (!(((num) >> 3) & 0b01))

#define FMT__NUM_PREFIX(num) ( \
	FMT__NUM_PREFIX_BIT(num) && ( \
		FMT__NUM_BASE(num) == FMT_NUM_BIN || \
		FMT__NUM_BASE(num) == FMT_NUM_HEX \
	) \
)

typedef struct { u32 repeat; i16 width; FmtNum num; i8 prec; } FmtOpt;

ENUM(FmtArgKind, u8) {
	// primitives
	// FMT_ARG_LIT,
	FMT_ARG_STR,
	FMT_ARG_U64,
	FMT_ARG_I64,
	FMT_ARG_F64,
	// custom
	FMT_ARG_MEM,
	FMT_ARG_COLOR,
};

typedef struct {
	union {
		Str str;
		u64 u64;
		i64 i64;
		f64 f64;
		u64 mem;
		Str color;
	} as;
	FmtOpt opt;
	FmtArgKind kind;
} FmtArg;

#define FMT_ARG(_kind, _as, _opt...) \
	((FmtArg){.as = {_as}, .opt = (FmtOpt){_opt}, .kind = (_kind)})

#define FMT_LIT(_lit, _opt...) FMT_ARG(FMT_ARG_STR, .str = STR(_lit), _opt)
#define FMT_STR(_str, _opt...) FMT_ARG(FMT_ARG_STR, .str =     _str,  _opt)
#define FMT_U64(_u64, _opt...) FMT_ARG(FMT_ARG_U64, .u64 =     _u64,  _opt)
#define FMT_I64(_i64, _opt...) FMT_ARG(FMT_ARG_I64, .i64 =     _i64,  _opt)
#define FMT_F64(_f64, _opt...) FMT_ARG(FMT_ARG_F64, .f64 =     _f64,  _opt)
#define FMT_MEM(_mem, _opt...) FMT_ARG(FMT_ARG_MEM, .mem =     _mem,  _opt)
#define FMT_COLOR_DYN(_color) FMT_ARG(FMT_ARG_COLOR, .color = _color)

#define FMT_COLOR(_color) FMT_COLOR_DYN(STR("\x1b[" _color "m"))

#define FMT_RESET  "0"
#define FMT_BOLD   "1"
#define FMT_DARK   "2"
#define FMT_ITALIC "3"

#define FMT_RED    "31"
#define FMT_GREEN  "32"
// #define FMT_PURPLE "33"
// #define FMT_PURPLE "34"
#define FMT_PURPLE "35"
#define FMT_CYAN   "36"
#define FMT_WHITE  "37"
#define FMT_ORANGE "38;5;214"
#define FMT_BLUE   "34"
#define FMT_GREY   "38;5;8"

#define FMT_LINE FMT_LIT("\n")

#define FMT_LOC(path, row, col) \
	FMT_COLOR(FMT_CYAN), FMT_LIT("--> "), \
	FMT_STR(path), FMT_LIT(":"), \
	FMT_U64(row), FMT_LIT(":"), \
	FMT_U64(col), FMT_LIT("\n")

#define FMT_LOC_DEBUG(call) \
	FMT_COLOR(FMT_GREY), FMT_LIT("->> "), \
	FMT_STR(STR_Z(call.path)), FMT_LIT(":"), \
	FMT_U64(call.line), FMT_LIT(" ("), \
	FMT_STR(STR_Z(call.func)), FMT_LIT(")"), \
	FMT_LIT("\n")

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define FMT_LOC_DEBUG_LIT \
	FMT_GREY_LIT "->> " \
	          __FILE__ ":" \
	STRINGIFY(__LINE__) "\n"

typedef struct {
	// Full view:
	// ```
	// @text1: some message
	// 100 | some.code += ...
	//            ^^^^ some thing here
	//    --> path/file.rk:100:10
	//
	// @text2: some message
	// 100 | some.code += ...
	//            ^^^^ some thing here
	//    --> path/file.rk:100:10
	// ```
	// Compact view:
	// ```
	// @text1: some message --> path/file.rk:100:10
	// @text2: some message --> path/file.rk:100:10
	// ```
	//
	// By default `@error`, `@warn` use Full view, `@note` use Compact view.
	// `@help` message is custom.
	bool full;
} FmtMessageView;

////////////////////////////////
// fmt over bump

typedef struct {
	Bump bump;
	bool color;
} FmtBump;

#define fmt_bump_write(fmt, args...) \
	fmt_bump_write_ex(fmt, (FmtArg[]){args}, LEN((FmtArg[]){args}))

Str fmt_bump_write_ex(FmtBump * fmt, FmtArg * ptr, usz len);

u32 fmt_bump_str(FmtBump * fmt, Str src, FmtOpt opt, Str * dst);
u32 fmt_bump_u64(FmtBump * fmt, u64 num, FmtOpt opt, Str * dst);
u32 fmt_bump_i64(FmtBump * fmt, i64 num, FmtOpt opt, Str * dst);

Str fmt_bump_as_str(FmtBump const * fmt);
bool fmt_bump_flush(FmtBump * fmt);
void fmt_bump_reset(FmtBump * fmt);

////////////////////////////////
// fmt over virtual bump

typedef struct {
	VirtBump virt;
	bool color;
} FmtVirt;

#define fmt_virt_write(virt, args...) \
	fmt_virt_write_ex(virt, (FmtArg[]){args}, LEN((FmtArg[]){args}))

Str fmt_virt_write_ex(FmtVirt * virt, FmtArg * ptr, usz len);

u32 fmt_virt_str(FmtVirt * virt, Str src, FmtOpt opt, Str * dst);
u32 fmt_virt_u64(FmtVirt * virt, u64 num, FmtOpt opt, Str * dst);
u32 fmt_virt_i64(FmtVirt * virt, i64 num, FmtOpt opt, Str * dst);

Str fmt_virt_as_str(FmtVirt const * fmt);
bool fmt_virt_flush(FmtVirt * fmt);
void fmt_virt_reset(FmtVirt * fmt);

////////////////////////////////
// generic api

#define fmt_write(fmt, args...) (_Generic(fmt, \
	FmtBump *: fmt_bump_write((void*)(fmt), args), \
	FmtVirt *: fmt_virt_write((void*)(fmt), args) \
))

#define fmt_as_str(fmt) (_Generic(fmt, \
	FmtBump const *: fmt_bump_as_str((void*)(fmt)), \
	FmtBump       *: fmt_bump_as_str((void*)(fmt)), \
	FmtVirt const *: fmt_virt_as_str((void*)(fmt)), \
	FmtVirt       *: fmt_virt_as_str((void*)(fmt))  \
))

#define fmt_flush(fmt) (_Generic(fmt, \
	FmtBump *: fmt_bump_flush((void*)(fmt)), \
	FmtVirt *: fmt_virt_flush((void*)(fmt)) \
))

#define fmt_reset(fmt) (_Generic(fmt, \
	FmtBump *: fmt_bump_reset((void*)(fmt)), \
	FmtVirt *: fmt_virt_reset((void*)(fmt)) \
))

#endif // !RK_FMT_H
