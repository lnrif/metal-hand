#ifndef STD_WINDOWS_EXIT_H
#define STD_WINDOWS_EXIT_H

#include "std/core.h"

#if WINDOWS && X86_64
// |================================================================================================|
// |> WINDOWS && X86_64                                                                             |
// |================================================================================================|

NO_RETURN IMPORT void ExitProcess(u32 code);

NO_RETURN IMPORT void ExitThread(u32 code);

#else
	#error "this file expected to use only in windows"
#endif

#endif // !STD_WINDOWS_EXIT_H
