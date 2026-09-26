#ifndef LINUX_CALL_H
#define LINUX_CALL_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#define LINUX_SYS_OPENAT       257
#define LINUX_SYS_FTRUNCATE     77
#define LINUX_SYS_MKDIRAT      258
#define LINUX_SYS_MMAP           9
#define LINUX_SYS_MUNMAP        11
#define LINUX_SYS_MADVISE       28
#define LINUX_SYS_MPROTECT      10
#define LINUX_SYS_MEMFD_CREATE 319
#define LINUX_SYS_NANOSLEEP     35

#define LINUX_SYS_READ           0
#define LINUX_SYS_WRITE          1
#define LINUX_SYS_READV         19
#define LINUX_SYS_WRITEV        20

#define LINUX_SYS_RECVMSG       47
#define LINUX_SYS_SENDMSG       46

#define LINUX_SYS_IOCTL         16
#define LINUX_SYS_CLOSE          3

#define LINUX_SYS_SOCKET        41
#define LINUX_SYS_CONNECT       42

#define LINUX_SYS_STATX        332
#define LINUX_STATX_SIZE 0x00000200U

#define LINUX_SYS_EXIT          60
#define LINUX_SYS_EXIT_GROUP   231

#define LINUX_SYS_GETRANDOM    318

static inline iptr syscall0(iptr num) {
	register iptr rax __asm__("rax") = num;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax)
		: "rcx", "r11", "memory"
	);

	return ret;
};

static inline iptr syscall1(iptr num, uptr a1) {
	register iptr rax __asm__("rax") = num;
	register uptr rdi __asm__("rdi") = a1;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax), "D"(rdi)
		: "rcx", "r11", "memory"
	);

	return ret;
};

static inline iptr syscall2(iptr num, uptr a1, uptr a2) {
	register iptr rax __asm__("rax") = num;
	register uptr rdi __asm__("rdi") = a1;
	register uptr rsi __asm__("rsi") = a2;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax), "D"(rdi), "S"(rsi)
		: "rcx", "r11", "memory"
	);

	return ret;
};

static inline iptr syscall3(iptr num, uptr a1, uptr a2, uptr a3) {
	register iptr rax __asm__("rax") = num;
	register uptr rdi __asm__("rdi") = a1;
	register uptr rsi __asm__("rsi") = a2;
	register uptr rdx __asm__("rdx") = a3;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax), "D"(rdi), "S"(rsi), "d"(rdx)
		: "rcx", "r11", "memory"
	);

	return ret;
};

static inline iptr syscall4(iptr num, uptr a1, uptr a2, uptr a3, uptr a4) {
	register iptr rax __asm__("rax") = num;
	register uptr rdi __asm__("rdi") = a1;
	register uptr rsi __asm__("rsi") = a2;
	register uptr rdx __asm__("rdx") = a3;
	register uptr r10 __asm__("r10") = a4;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax), "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10)
		: "rcx", "r11", "memory"
	);

	return ret;
};

static inline iptr syscall5(iptr num, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5) {
	register iptr rax __asm__("rax") = num;
	register uptr rdi __asm__("rdi") = a1;
	register uptr rsi __asm__("rsi") = a2;
	register uptr rdx __asm__("rdx") = a3;
	register uptr r10 __asm__("r10") = a4;
	register uptr r8  __asm__("r8")  = a5;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax), "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8)
		: "rcx", "r11", "memory"
	);

	return ret;
};

static inline iptr syscall6(iptr num, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5, uptr a6) {
	register iptr rax __asm__("rax") = num;
	register uptr rdi __asm__("rdi") = a1;
	register uptr rsi __asm__("rsi") = a2;
	register uptr rdx __asm__("rdx") = a3;
	register uptr r10 __asm__("r10") = a4;
	register uptr r8  __asm__("r8")  = a5;
	register uptr r9  __asm__("r9")  = a6;
	iptr ret;

	__asm__ volatile (
		"syscall"
		: "=a"(ret)
		: "a"(rax), "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8), "r"(r9)
		: "rcx", "r11", "memory"
	);

	return ret;
};

#elif LINUX && AARCH64
// |================================================================================================|
// |> LINUX && ARM64                                                                               |
// |================================================================================================|

#define LINUX_SYS_OPENAT        56
#define LINUX_SYS_FTRUNCATE     46
#define LINUX_SYS_MKDIRAT       34
#define LINUX_SYS_MMAP         222
#define LINUX_SYS_MUNMAP       215
#define LINUX_SYS_MADVISE      233
#define LINUX_SYS_MPROTECT     226
#define LINUX_SYS_MEMFD_CREATE 279
#define LINUX_SYS_NANOSLEEP    101

#define LINUX_SYS_READ          63
#define LINUX_SYS_WRITE         64
#define LINUX_SYS_READV         65
#define LINUX_SYS_WRITEV        66

#define LINUX_SYS_RECVMSG      212
#define LINUX_SYS_SENDMSG      211

#define LINUX_SYS_IOCTL         29
#define LINUX_SYS_CLOSE         57

#define LINUX_SYS_SOCKET       198
#define LINUX_SYS_CONNECT      203

#define LINUX_SYS_STATX        332
#define LINUX_STATX_SIZE 0x00000200U

#define LINUX_SYS_EXIT          93
#define LINUX_SYS_EXIT_GROUP    94

#define LINUX_SYS_GETRANDOM    278

static inline iptr syscall0(iptr num) {
	register iptr x8 __asm__("x8") = num;
	register iptr x0 __asm__("x0");

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

static inline iptr syscall1(iptr num, uptr a1) {
	register iptr x8 __asm__("x8") = num;
	register uptr x0 __asm__("x0") = a1;

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

static inline iptr syscall2(iptr num, uptr a1, uptr a2) {
	register iptr x8 __asm__("x8") = num;
	register uptr x0 __asm__("x0") = a1;
	register uptr x1 __asm__("x1") = a2;

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x1), "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

static inline iptr syscall3(iptr num, uptr a1, uptr a2, uptr a3) {
	register iptr x8 __asm__("x8") = num;
	register uptr x0 __asm__("x0") = a1;
	register uptr x1 __asm__("x1") = a2;
	register uptr x2 __asm__("x2") = a3;

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x1), "r"(x2), "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

static inline iptr syscall4(iptr num, uptr a1, uptr a2, uptr a3, uptr a4) {
	register iptr x8 __asm__("x8") = num;
	register uptr x0 __asm__("x0") = a1;
	register uptr x1 __asm__("x1") = a2;
	register uptr x2 __asm__("x2") = a3;
	register uptr x3 __asm__("x3") = a4;

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x1), "r"(x2), "r"(x3), "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

static inline iptr syscall5(iptr num, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5) {
	register iptr x8 __asm__("x8") = num;
	register uptr x0 __asm__("x0") = a1;
	register uptr x1 __asm__("x1") = a2;
	register uptr x2 __asm__("x2") = a3;
	register uptr x3 __asm__("x3") = a4;
	register uptr x4 __asm__("x4") = a5;

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

static inline iptr syscall6(iptr num, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5, uptr a6) {
	register iptr x8 __asm__("x8") = num;
	register uptr x0 __asm__("x0") = a1;
	register uptr x1 __asm__("x1") = a2;
	register uptr x2 __asm__("x2") = a3;
	register uptr x3 __asm__("x3") = a4;
	register uptr x4 __asm__("x4") = a5;
	register uptr x5 __asm__("x5") = a6;

	__asm__ volatile (
		"svc 0"
		: "+r"(x0)
		: "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
		: "memory"
	);

	return (iptr)x0;
};

#endif

#endif // !LINUX_CALL_H
