#ifndef STD_WINDOWS_VIRT_H
#define STD_WINDOWS_VIRT_H

#include "std/core.h"

#if WINDOWS && X86_64
// |================================================================================================|
// |> WINDOWS && X86_64                                                                             |
// |================================================================================================|

#define WINDOWS_PAGE_NOACCESS     0x01
#define WINDOWS_PAGE_READONLY     0x02
#define WINDOWS_PAGE_READWRITE    0x04
#define WINDOWS_PAGE_EXECUTE_READ 0x20

#define WINDOWS_MEM_COMMIT  0x00001000
#define WINDOWS_MEM_RESERVE 0x00002000
#define WINDOWS_MEM_RELEASE 0x00008000

extern void* VirtualAlloc(void* lpAddress, size_t dwSize, unsigned long flAllocationType, unsigned long flProtect);
extern int VirtualFree(void* lpAddress, size_t dwSize, unsigned long dwFreeType);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_WINDOWS_VIRT_H
