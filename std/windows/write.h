#ifndef STD_WINDOWS_WRITE_H
#define STD_WINDOWS_WRITE_H

#include "std/core.h"

#if WINDOWS && X86_64
// |================================================================================================|
// |> WINDOWS && X86_64                                                                             |
// |================================================================================================|

IMPORT b32 WriteFile(void * hFile, void const * lpBuffer, u32 nNumberOfBytesToWrite, u32* lpNumberOfBytesWritten, void* lpOverlapped);
IMPORT void * GetStdHandle(u32 nStdHandle);

typedef void * WindowsHandle;
#define WINDOWS_INVALID_HANDLE ((void*)-1)

WindowsHandle windows_handle(u32 id);

#define WINDOWS_INPUT_HANDLE  ((u32)-10)
#define WINDOWS_OUTPUT_HANDLE ((u32)-11)
#define WINDOWS_ERROR_HANDLE  ((u32)-12)

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_WINDOWS_WRITE_H
