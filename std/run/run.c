#include "std/run/run.h"
#include "std/flow/core.h"
#include "std/proc/exit.h"

#if defined(__linux__) && defined(__x86_64__)
// |================================================================================================|
// |> LINUX (x86_64)                                                                                |
// |================================================================================================|

USED void _start_c(uptr * stack) {
	uptr raw_argc = stack[0];

	u32 argc = (u32)raw_argc;
	u8 const * const * argv = (u8 const * const *)&stack[1];
	u8 const * const * envp = (u8 const * const *)&stack[1 + argc + 1];

	flow_init();

	Flow_Stat * stat = FLOW_STAT("run {Run} -> {}");

	flow_enter(stat, FLOW_LOC); run((Run){
		.env      = envp,
		.args     = argv,
		.args_len = argc,
	});

	//
	// Stream out = stream_output();
	// if (!stream_is_terminal(&out)) proc_exit(1);
	//
	// if (!stream_enable_ansi(&out)) {
	// 	stream_write_lit(&out, "Failed to enable ANSI\n\n");
	// 	proc_exit(1);
	// };
	//
	// if (!term_enable_utf8()) {
	// 	stream_write_lit(&out, "Failed to enable UTF-8\n\n");
	// 	proc_exit(1);
	// };
	//
	//
	// flow_show_calls(&out);
	//

	flow_leave(stat);
	flow_free();

	proc_exit(0);
};

NAKED void _start(void) {
	__asm__ volatile (
		"mov %rsp, %rdi\n"
		"and $-16, %rsp\n"
		"sub $8, %rsp\n"
		"jmp _start_c\n"
	);
};

#elif defined(_WIN32) && defined(__x86_64__)
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
