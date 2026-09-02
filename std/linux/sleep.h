#ifndef STD_LINUX_SLEEP_H
#define STD_LINUX_SLEEP_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "std/linux/call.h"

static inline i64 linux_nanosleep(i64 sec, i64 nsec) {
	struct { i64 sec; i64 nsec; } req = {sec, nsec};
	return syscall2(35, (uptr)&req, 0);
};

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_LINUX_SLEEP_H
