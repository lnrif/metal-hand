#ifndef LINUX_EXIT_H
#define LINUX_EXIT_H

#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

// [exit]
NO_RETURN void linux_thread_exit(u8 status);
// [exit_group]
NO_RETURN void linux_proc_exit(u8 status);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !LINUX_EXIT_H
