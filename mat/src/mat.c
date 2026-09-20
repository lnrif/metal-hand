#include "mat/src/mat.h"

// |================================================================================================|
// |> Matrix                                                                                        |

b8 mat_fma_col(Col dst, Mat w, Col x, Col b) {
	if (w.row != dst.len || w.col != x.len || w.row != b.len) return false;

	for (u64 i = 0; i < dst.len; i += 1) {
		f32 a = 0.0;
		for (u64 j = 0; j < x.len; j += 1) {
			a += w.ptr[i * w.col + j] * x.ptr[j];
		};
		dst.ptr[i] = a + b.ptr[i];
	};

	return true;
};

b8 mat_fmt_item(Fmt * fmt, f32 x, MatStyle style) {
	FmtArg color = FMT_GREY;
	if (x < 0) {
		color = FMT_BLUE;
	} else if (x > 0) {
		color = FMT_RED;
	};
	return FMT(fmt, color, FMT_F64(x, .prec = style.prec, .opt = FMT_N_SIGN));
};

b8 mat_fmt_item_sign(Fmt * fmt, f32 x) {
	if (x < 0) {
		return FMT(fmt, FMT_BLUE, FMT_LIT("-"));
	} else if (x > 0) {
		return FMT(fmt, FMT_RED, FMT_LIT("+"));
	} else {
		return FMT(fmt, FMT_GREY, FMT_LIT("="));
	};
};

b8 mat_fmt_ex(Fmt * fmt, Mat mat, MatStyle style) {
	u32 offset = 0;

	if (str_sane(style.name)) {
		offset = (u32)(style.name.len + S(" = ").len);
		FMT(fmt,
			FMT_CYAN, FMT_STR(style.name),
			FMT_YELLOW, FMT_LIT(" = "),
		);
	};

	for (u64 i = 0; i < mat.row; i += 1) {
		Str beg = (i == 0)           ? S("[[")   : S(" [");
		Str end = (i == mat.row - 1) ? S("]]\n") : S("],\n");
		u32 repeat = (i == 0) ? 0 : offset;
		FMT(fmt, FMT_REPEAT(' ', repeat), FMT_YELLOW, FMT_STR(beg));

		for (u64 j = 0; j < mat.col - 1; j += 1) {
			f32 x = mat.ptr[i * mat.col + j];
			mat_fmt_item(fmt, x, style);
			FMT(fmt, FMT_YELLOW, FMT_LIT(", "));
		};

		f32 x = mat.ptr[i * mat.col + mat.col - 1];
		mat_fmt_item(fmt, x, style);

		FMT(fmt, FMT_YELLOW, FMT_STR(end));
	};

	return true;
};

b8 lin_eval(Lin * lin);
b8 lin_learn(Lin * lin);
b8 lin_step(Lin * lin);

b8 relu_eval(Relu * relu);
b8 relu_learn(Relu * relu);
b8 relu_step(Relu * relu);

b8 add_eval(Add * add);
b8 add_learn(Add * add);
b8 add_step(Add * add);

Insts insts_from_slice(Inst * ptr, u32 len);
b8    insts_push(Insts * insts, Inst item);
void  insts_reset(Insts * insts);

