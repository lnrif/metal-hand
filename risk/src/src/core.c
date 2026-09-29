#include "risk/src/src/core.h"

SrcDot src_dot(Str src, SrcDot from, u32 at) {
	Str seg = str_sub(src, from.at, at);

	SrcDot dot = (SrcDot){
		.row = from.row, .col = from.col,
		.at = at,
	};

	for (u64 i = 0; i < seg.len; i += 1) {
		if (seg.raw[i] == '\n') {
			dot.row += 1;
			dot.col = 1;
		} else {
			dot.col += 1;
		};
	};

	return dot;
};
