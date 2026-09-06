#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "linux/fs.h"
#include "linux/call.h"

// [openat]
i64 linux_open_at(LinuxFd dir, u8z const * path, u32 flags, u32 mode) {
	return syscall4(257, (uptr)dir, (uptr)path, (uptr)flags, (uptr)mode);
};

// [ftruncate]
i64 linux_ftruncate(LinuxFd fd, u64 length) {
	return syscall2(77, (uptr)fd, (uptr)length);
};


#endif
