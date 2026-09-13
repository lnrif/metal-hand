#include "sys/sys.h"

bool sys_write(u8 const * ptr, usz len) {
    if (len == 0) return TRUE;

#if defined(_WIN32) || defined(_WIN64)
	extern void * __stdcall GetStdHandle(int nStdHandle);
	extern int    __stdcall WriteFile(
		void * hFile,
		const void* lpBumpfer,
		unsigned int nNumberOfBytesToWrite,
		unsigned int* lpNumberOfBytesWritten,
		void* lpOverlapped
	);

	void* stdout_handle = GetStdHandle(-12);
	unsigned int written = 0;
	return WriteFile(stdout_handle, ptr, (unsigned int)len, &written, NULL) && (written == len);

#else
	extern long write(int fd, const void *buf, usz count);

	long result = write(2, ptr, len);
	return (result >= 0 && (usz)result == len);
#endif
};
