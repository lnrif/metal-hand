#ifndef LINUX_SLEEP_H
#define LINUX_SLEEP_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

// [nanosleep]
i64 linux_nanosleep(i64 sec, i64 nsec);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !LINUX_SLEEP_H
