#ifndef RK_SRC_H
#define RK_SRC_H

#include "fs/fs.h"
#include "fmt/fmt.h"
#include "str/str.h"

typedef struct {
	u32 pos;
	u32 len;
} SrcSpan;

typedef struct {
	u32 row;
	u32 col;
} SrcDot;

typedef struct {
	union { struct { u32 pos; u32 len; }; SrcSpan span; };
	union { struct { u32 row; u32 col; }; SrcDot  dot;  };
} SrcLoc;

typedef struct {
	STR_EMBED(str_z, ptr_z, len);
	FsPathZ path;
} Src;

SrcDot src_eval_dot(Str src, usz at);


#define src_load(out, bump, src, path) src_load_ex(CALL, out, bump, src, path)
bool src_load_ex(CallLoc call, FmtVirt * out, VirtBump * bump, Src * src, FsPathZ path);

#endif // !RK_SRC_H
