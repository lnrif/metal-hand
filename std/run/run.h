#ifndef STD_RUN_H
#define STD_RUN_H

#include "std/core.h"

#if defined(__linux__) && defined(__x86_64__)
// |================================================================================================|
// |> LINUX (x86_64)                                                                                |
// |================================================================================================|

typedef struct {
	u8 const * const * env;
	u8 const * const * args;
	u32 args_len;
} Run;

#elif defined(_WIN32) && defined(__x86_64__)
// |================================================================================================|
// |> WINDOWS (x86_64)                                                                              |
// |================================================================================================|

typedef void * Run;

#endif

void run(Run run);

#if defined(__linux__) && defined(__x86_64__)
// |================================================================================================|
// |> LINUX (x86_64)                                                                                |
// |================================================================================================|

NAKED void _start(void);

#elif defined(_WIN32) && defined(__x86_64__)
// |================================================================================================|
// |> WINDOWS (x86_64)                                                                              |
// |================================================================================================|

USED void _start(void);

#else

#error "unsuppoted OS"

#endif

// __declspec(dllimport) wchar_t* __stdcall GetCommandLineW(void);
// __declspec(dllimport) wchar_t** __stdcall CommandLineToArgvW(const wchar_t* lpCmdLine, int* pNumArgs);
// __declspec(dllimport) wchar_t* __stdcall GetEnvironmentStringsW(void);
// __declspec(dllimport) void     __stdcall ExitProcess(unsigned int uExitCode);
#endif // !STD_RUN_H
