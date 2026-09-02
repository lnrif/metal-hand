#include "std/proc/exit.h"

#if LINUX
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "std/linux/exit.h"

NO_RETURN void proc_exit(u8 status) {
	linux_proc_exit(status);
};

#elif WINDOWS
// |================================================================================================|
// |> WINDOWS                                                                                       |
// |================================================================================================|

#include "std/windows/exit.h"

NO_RETURN void process_exit(u8 status) {
	ExitProcess((u32)status);
};

#else
	#error "not supported"
#endif

