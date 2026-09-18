#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "linux/sleep.h"
#include "linux/call.h"

// [nanosleep]
i64 linux_nanosleep(i64 sec, i64 nsec) {
	struct { i64 sec; i64 nsec; } req = {sec, nsec};
	return syscall2(LINUX_SYS_NANOSLEEP, (uptr)&req, 0);
};

#endif // !LINUX_SYS_CALL
