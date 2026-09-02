#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "std/linux/exit.h"
#include "std/linux/call.h"

NO_RETURN void linux_thread_exit(u8 status) {
	syscall1(60, (uptr)status);
	UNREACHABLE;
};

NO_RETURN void linux_proc_exit(u8 status) {
	syscall1(231, (uptr)status);
	UNREACHABLE;
};

#endif
