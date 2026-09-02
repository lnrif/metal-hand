#ifndef STD_LINUX_FS_H
#define STD_LINUX_FS_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "std/core.h"
#include "std/linux/write.h"

#define LINUX_FS_READ_ONLY  00
#define LINUX_FS_WRITE_ONLY 01
#define LINUX_FS_READ_WRITE 02
#define LINUX_FS_CREATE     0100
#define LINUX_FS_TRUNCATE   01000

i64 linux_open_at(LinuxFd dir, u8z const * path, u32 flags, u32 mode);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_LINUX_FS_H
