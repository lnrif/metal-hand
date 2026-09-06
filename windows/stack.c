#ifndef STD_WINDOWS_STACK_H
#define STD_WINDOWS_STACK_H

#include "std/core.h"

#if WINDOWS && X86_64
// |================================================================================================|
// |> WINDOWS && X86_64                                                                             |
// |================================================================================================|

// #define ALIAS(name) __attribute__((alias(#name)))

USED NAKED void ___chkstk_ms(void) {
	__asm__ volatile (
		"push %rcx\n"
		"push %rax\n"
		"mov %rsp, %rcx\n"
		"add $16, %rcx\n"

	"1:\n"
		"cmp $4096, %rax\n"
		"jbe 2f\n"

		"sub $4096, %rcx\n"
		"test %eax, (%rcx)\n"
		"sub $4096, %rax\n"
		"jmp 1b\n"

	"2:\n"
		"sub %rax, %rcx\n"
		"test %eax, (%rcx)\n"

		"pop %rax\n"
		"pop %rcx\n"
		"ret\n"
	);
};

#endif

#endif // !STD_WINDOWS_STACK_H
