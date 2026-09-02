#ifndef STD_LINUX_CALL_H
#define STD_LINUX_CALL_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "std/core.h"

static inline iptr syscall0(iptr num) {
	iptr ret;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num) : "rcx", "r11", "memory");
	return ret;
};

static inline iptr syscall1(iptr num, uptr a1) {
	iptr ret;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(a1) : "rcx", "r11", "memory");
	return ret;
};

static inline iptr syscall2(iptr num, uptr a1, uptr a2) {
	iptr ret;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
	return ret;
};

static inline iptr syscall3(iptr num, uptr a1, uptr a2, uptr a3) {
	iptr ret;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(a1), "S"(a2), "d"(a3) : "rcx", "r11", "memory");
	return ret;
};

static inline iptr syscall4(iptr num, uptr a1, uptr a2, uptr a3, uptr a4) {
	iptr ret;
	register uptr r10 __asm__("r10") = a4;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10) : "rcx", "r11", "memory");
	return ret;
};

static inline iptr syscall5(iptr num, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5) {
	iptr ret;
	register uptr r10 __asm__("r10") = a4;
	register uptr r8  __asm__("r8")  = a5;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
	return ret;
};

static inline iptr syscall6(iptr num, uptr a1, uptr a2, uptr a3, uptr a4, uptr a5, uptr a6) {
	iptr ret;
	register uptr r10 __asm__("r10") = a4;
	register uptr r8  __asm__("r8")  = a5;
	register uptr r9  __asm__("r9")  = a6;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
	return ret;
};

#endif

#endif // !STD_LINUX_CALL_H
