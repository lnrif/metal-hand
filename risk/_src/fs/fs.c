
#ifdef _WIN32
	#error "`fs` not implemented for [Windows]"
#elif __linux__
	#include "fs/_linux.c"
#else
	#error "`fs` not implemented for this platform"
#endif

