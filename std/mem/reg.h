#ifndef STD_MEM_REG_H
#define STD_MEM_REG_H

#include "std/core.h"
#include <stdalign.h>

// |================================================================================================|
// |> REGION                                                                                        |
// |================================================================================================|

typedef enum: u64 { REG_C_OOM, REG_C_INVALID_ARGS } RegCode;

typedef struct {
	union { uptr ptr; void * any; u8 * raw; b64 is_valid; };
	union { u64 len; RegCode code; };
} Reg;

// if (!reg.valid) return reg.code;

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
#define REG_ARR(arr) REG((arr), sizeof(arr))
#define REG_VEC(_ptr, _len) ((Reg){.any = (_ptr), .len = (_len) * sizeof(typeof(*(_ptr)))})

#define REG_NIL REG(0, 0)
#define REG_SLICE(_ptr, _len) ((Reg){.any = (_ptr), .len = (_len) * sizeof(typeof(*(_ptr)))})

// #define REG_BEG(reg) ((reg)->ptr)
// #define REG_END(reg) ((reg)->ptr + (reg)->len)
// #define REG_LEN(reg) ((reg)->len)

// |================================================================================================|
// |> req                                                                                           |

typedef enum: u8 { REG_DIR_DOWN, REG_DIR_UP } RegDir;

typedef struct {
	u64 len;
	u32 align;
	RegDir dir;
} RegReq;

#define REG_REQ(_len, _align, _dir) ((RegReq){.len = (_len), .align = (_align), .dir = (_dir)})
#define REG_REQ_VEC(T, len, dir) REG_REQ((len) * sizeof(T), alignof(T), dir)
#define REG_REQ_FREE REG_REQ(0, 0, REG_DIR_UP)

// |================================================================================================|
// |> call                                                                                          |

typedef void * RegCtx;
typedef Reg (*RegApi)(RegCtx ctx, Reg reg, RegReq req);

// |================================================================================================|
// |> man                                                                                           |

typedef struct { RegCtx ctx; RegApi api; } RegMan;

#define REG_MAN(_ctx, _api) ((RegMan){.ctx = (_ctx), .api = (_api)})
#define REG_MAN_NIL REG_MAN(0, 0)

// |================================================================================================|
// |> wrapper                                                                                       |

typedef struct { Reg reg; RegReq req; } RegUpd;
#define REG_UPD(_reg, _req) ((RegUpd){.reg = (_reg), .req = (_req)})

RegUpd reg_upd_arr_ex(
	u64 item_size, u32 item_align,
	void * items, u64 count, u64 new_count,
	RegDir dir
);

#define REG_UPD_ARR(ptr, len, new_len, dir) \
	reg_upd_arr_ex( \
		sizeof(*(ptr)), alignof(typeof(*(ptr))), \
		(ptr), (len), (new_len), (dir) \
	)

#define reg_upd_arr(man, ptr, len, new_len, dir) \
	reg_upd(man, REG_UPD_ARR(ptr, len, new_len, dir))

#define reg_alloc(man, T, count) \
	reg_upd(man, REG_UPD(REG_NIL, REG_REQ(sizeof(T) * (count), alignof(T), REG_DIR_UP)))

#define reg_free(man, ptr, count) \
	reg_upd(man, REG_UPD(REG_VEC(ptr, count), REG_REQ_FREE))

Reg reg_upd(RegMan man, RegUpd upd);

#endif // !STD_MEM_REG_H

