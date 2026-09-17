#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "linux/exit.h"
#include "linux/call.h"

NO_RETURN void linux_thread_exit(u8 status) {
	syscall1(60, (uptr)status);
	UNREACHABLE;
};

NO_RETURN void linux_proc_exit(u8 status) {
	syscall1(231, (uptr)status);
	UNREACHABLE;
};

#endif
