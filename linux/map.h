#ifndef LINUX_SLEEP_H
#define LINUX_SLEEP_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "linux/write.h"

#define LINUX_MAP_PROT_NONE  0x0
#define LINUX_MAP_PROT_READ  0x1
#define LINUX_MAP_PROT_WRITE 0x2
#define LINUX_MAP_SHARED     0x01
#define LINUX_MAP_PRIVATE    0x02
#define LINUX_MAP_ANONYMOUS  0x20

// [mmap]
void * linux_mmap(void * addr, u64 length, i32 prot, i32 flags, i32 fd, u64 offset);
// [munmap]
iptr linux_munmap(void * addr, u64 length);
// [madvice]
iptr linux_madvice(void * addr, u64 length, i32 advice);

// [memfd_create]
LinuxFd linux_memfd_create(char const * name, u32 flags);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !LINUX_SLEEP_H

