#include "std/core.h"

#if WINDOWS && X86_64
// |================================================================================================|
// |> WINDOWS && X86_64                                                                             |
// |================================================================================================|

#include "std/windows/write.h"

WindowsHandle windows_handle(u32 id) {
	return GetStdHandle(id);
};

#endif


