#include "std/core.h"

u64 max_u64(u64 * ptr, u64 len) {
	if (len == 0) return 0;
	u64 max = ptr[0];
	for (u64 i = 1; i < len; i += 1) if (max < ptr[i]) max = ptr[i];
	return max;
};

