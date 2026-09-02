#include "std/mem/reg.h"

// |================================================================================================|
// |> wrapper                                                                                       |

RegUpd reg_upd_arr_ex(
	u64 item_size, u32 item_align,
	void * items, u64 count, u64 new_count,
	RegDir dir
) {
	return (RegUpd){
		.reg = REG((uptr)items, count * item_size),
		.req = REG_REQ(new_count * item_size, item_align, dir),
	};
};

Reg reg_upd(RegMan man, RegUpd upd) {
	return (man.api != 0) ? man.api(man.ctx, upd.reg, upd.req) : REG_NIL;
};

