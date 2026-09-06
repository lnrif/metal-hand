#include "linux/sleep.h"
#include "linux/call.h"

// [nanosleep]
i64 linux_nanosleep(i64 sec, i64 nsec) {
	struct { i64 sec; i64 nsec; } req = {sec, nsec};
	return syscall2(35, (uptr)&req, 0);
};

