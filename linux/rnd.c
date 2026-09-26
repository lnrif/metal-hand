#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "linux/rnd.h"
#include "linux/call.h"

// [getrandom]
iptr linux_getrandom(void * buf, u64 len) {
	return syscall3(LINUX_SYS_GETRANDOM, (uptr)buf, (uptr)len, 0);
};

#endif // !LINUX_SYS_CALL
