#ifndef RK_SRC_H
#define RK_SRC_H

// #include "fs/fs.h"
#include "std/fmt/core.h"
#include "std/str/core.h"

typedef struct { u32 pos; u32 len; } SrcSpan;
typedef struct { u32 row; u32 col; u32 at; } SrcDot;

#define SRC_DOT_NIL ((SrcDot){.row = 1, .col = 1, .at = 0})

typedef struct {
	union { struct { u32 pos; u32 len;         }; SrcSpan span; };
	union { struct { u32 row; u32 col; u32 at; }; SrcDot  dot;  };
} SrcLoc;

typedef struct {
	STR_Z_EMBED(str_z, str, ptr, any, raw, len);
	Str path;
} Src;

SrcDot src_dot(Str src, SrcDot from, u32 at);

// #define src_load(out, bump, src, path) src_load_ex(CALL, out, bump, src, path)
// bool src_load_ex(CallLoc call, FmtVirt * out, VirtBump * bump, Src * src, FsPathZ path);

#endif // !RK_SRC_H
