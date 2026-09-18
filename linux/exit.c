#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "linux/exit.h"
#include "linux/call.h"

NO_RETURN void linux_thread_exit(u8 status) {
	syscall1(LINUX_SYS_EXIT, (uptr)status);
	UNREACHABLE;
};

NO_RETURN void linux_proc_exit(u8 status) {
	syscall1(LINUX_SYS_EXIT_GROUP, (uptr)status);
	UNREACHABLE;
};

#endif
