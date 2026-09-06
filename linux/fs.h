#ifndef LINUX_FS_H
#define LINUX_FS_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "linux/write.h"

#define LINUX_FS_READ_ONLY  00
#define LINUX_FS_WRITE_ONLY 01
#define LINUX_FS_READ_WRITE 02
#define LINUX_FS_CREATE     0100
#define LINUX_FS_TRUNCATE   01000

// [openat]
i64 linux_openat(LinuxFd dir, u8z const * path, u32 flags, u32 mode);
// [ftruncate]
i64 linux_ftruncate(LinuxFd fd, u64 length);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !LINUX_FS_H
