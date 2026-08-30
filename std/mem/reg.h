#ifndef STD_MEM_REG_H
#define STD_MEM_REG_H

#include "std/core.h"

// |================================================================================================|
// |> REGION                                                                                        |
// |================================================================================================|

typedef struct {
	union { uptr ptr; void * any; u8 * raw; };
	u64 len;
} Reg;

#define REG_EMBED(reg, ptr, any, raw, len) \
	union { \
		Reg reg; \
		struct { \
			union { uptr ptr; void * any; u8 * raw; }; \
			u64 len; \
		}; \
	}

// |================================================================================================|
// |> [Reg]: helpers                                                                                |

#define REG(_ptr, _len) ((Reg){.ptr = (_ptr), .len = (_len)})

#define REG_NIL REG(0, 0)
#define REG_ARR(arr) REG((arr), sizeof(arr))

#define REG_BEG(reg) ((reg)->ptr)
#define REG_END(reg) ((reg)->ptr + (reg)->len)
#define REG_LEN(reg) ((reg)->len)

// |================================================================================================|
// |> [Reg]: call                                                                                   |

typedef struct {
	u64 len;
	u32 align;
	i8  dir;
} RegArgs;

#define REG_RESIZE(_len, _align, _dir...) \
	((RegArgs){.len = (_len), .align = (_align), _dir})

#define REG_FREE REG_RESIZE(0, 0, .dir = 0)

typedef void * RegCtx;
typedef Reg (*RegUpd)(RegCtx ctx, Reg prev, RegArgs args);

typedef struct { RegCtx ctx; RegUpd upd; } RegVT;

#define REG_VT(_ctx, _upd) ((RegVT){.ctx = (_ctx), .upd = (_upd)})
#define REG_VT_NIL REG_VT(0, 0)

static inline Reg reg_upd(RegVT vt, Reg prev, RegArgs args) {
	return (vt.upd != 0) ? vt.upd(vt.ctx, prev, args) : REG_NIL;
};

#endif // !STD_MEM_REG_H

