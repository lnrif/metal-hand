
#ifndef LINUX_RND_H
#define LINUX_RND_H

#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

// [getrandom]
iptr linux_getrandom(void * buf, u64 len);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !LINUX_RND_H
