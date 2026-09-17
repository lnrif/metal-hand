#include "std/core.h"

#if LINUX_SYSCALL
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "linux/map.h"
#include "linux/call.h"

// [mmap]
iptr linux_mmap(uptr addr, u64 length, i32 prot, i32 flags, i32 fd, u64 offset) {
	return syscall6(9, (uptr)addr, (uptr)length, (uptr)prot, (uptr)flags, (uptr)fd, (uptr)offset);
};

// [munmap]
iptr linux_munmap(uptr addr, u64 length) {
	return syscall2(11, (uptr)addr, (uptr)length);
};

// [madvice]
iptr linux_madvice(uptr addr, u64 length, i32 advice) {
	return syscall3(28, (uptr)addr, (uptr)length, (uptr)advice);
};

// [mprotect]
iptr linux_mprotect(uptr addr, u64 length, i32 prot) {
	return syscall3(10, (uptr)addr, (uptr)length, (uptr)prot);
};

// [memfd_create]
LinuxFd linux_memfd_create(char const * name, u32 flags) {
	return (LinuxFd)syscall2(319, (uptr)name, (uptr)flags);
};

#endif // !LINUX_SYSCALL
