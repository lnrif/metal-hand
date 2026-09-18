#include "std/run/run.h"
#include "std/proc/exit.h"

#if LINUX && (X86_64 || AARCH64)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

void USED _start_c(uptr * stack) {
	uptr raw_argc = stack[0];

	u32 argc = (u32)raw_argc;
	u8 const * const * argv = (u8 const * const *)&stack[1];
	u8 const * const * envp = (u8 const * const *)&stack[1 + argc + 1];

	run((Run){
		.env      = envp,
		.args     = argv,
		.args_len = argc,
	});

	proc_exit(0);
};

#if X86_64

NAKED void _start(void) {
	__asm__ volatile (
		"mov %rsp, %rdi\n"
		"and $-16, %rsp\n"
		"sub $8, %rsp\n"
		"jmp _start_c\n"
	);
};

#elif AARCH64

NAKED void _start(void) {
	__asm__ volatile (
		"mov x0, sp\n"
		"bic x1, x0, #15\n"
		"mov sp, x1\n"
		"mov x29, #0\n"
		"mov x30, #0\n"
		"b _start_c\n"
	);
};

#else
	#error "unsupported"
#endif

#elif WINDOWS && X86_64
// |================================================================================================|
// |> WINDOWS (x86_64)                                                                              |
// |================================================================================================|

USED void _start(void) {
	run((Run){});
	PANIC_RAW("function `run` should never return");
};

#else

#error "unsupported"

#endif
