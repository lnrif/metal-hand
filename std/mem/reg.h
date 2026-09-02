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

RegUpd reg_upd_arr_ex(
	u64 item_size, u32 item_align,
	void * items, u64 count, u64 new_count,
	RegDir dir
);

#define REG_UPD_ARR(_ptr, _len, _new_len, _dir) \
	reg_upd_arr_ex( \
		sizeof(*(_ptr)), alignof(*(_ptr)), \
		(_ptr), (_len), (_new_len), (_dir) \
	)

#define reg_upd_arr(_man, _ptr, _len, _new_len, _dir) \
	reg_upd(_man, REG_UPD_ARR(_ptr, _len, _new_len, _dir))

Reg reg_upd(RegMan man, RegUpd upd);

#endif // !STD_MEM_REG_H

