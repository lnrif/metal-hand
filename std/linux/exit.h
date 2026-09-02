#ifndef STD_LINUX_EXIT_H
#define STD_LINUX_EXIT_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

// [exit]
NO_RETURN void linux_thread_exit(u8 status);
// [exit_group]
NO_RETURN void linux_proc_exit(u8 status);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_LINUX_EXIT_H
