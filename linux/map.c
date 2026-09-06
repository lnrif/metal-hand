#include "linux/map.h"
#include "linux/call.h"

// [mmap]
void * linux_mmap(void * addr, u64 length, i32 prot, i32 flags, i32 fd, u64 offset) {
	return (void*)syscall6(9, (uptr)addr, (uptr)length, (uptr)prot, (uptr)flags, (uptr)fd, (uptr)offset);
};

// [munmap]
iptr linux_munmap(void * addr, u64 length) {
	return syscall2(11, (uptr)addr, (uptr)length);
};

// [madvice]
iptr linux_madvice(void * addr, u64 length, i32 advice) {
	return syscall3(28, (uptr)addr, (uptr)length, (uptr)advice);
};

// [memfd_create]
LinuxFd linux_memfd_create(char const * name, u32 flags) {
	return (LinuxFd)syscall2(319, (uptr)name, (uptr)flags);
};


