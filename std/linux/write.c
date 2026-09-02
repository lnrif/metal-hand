#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "std/str/str.h"
#include "std/linux/write.h"
#include "std/linux/call.h"

// |================================================================================================|
// |> Fd (fd)                                                                                       |

// [read]
i64 linux_read(LinuxFd handle, void * buf, u64 count) {
	return syscall3(0, (uptr)handle, (uptr)buf, (uptr)count);
};

// [write]
i64 linux_write(LinuxFd handle, void const * ptr, u64 len) {
	return syscall3(1, (uptr)handle, (uptr)ptr, (uptr)len);
};

i64 linux_write_str(LinuxFd handle, Str s) {
	return linux_write(handle, s.ptr, s.len);
};

// |================================================================================================|
// |> Vec (iovec)                                                                                   |

// [readv]
i64 linux_read_vec(LinuxFd handle, LinuxVec * ptr, u32 len) {
	return syscall3(19, (uptr)handle, (uptr)ptr, (uptr)len);
};

// [writev]
i64 linux_write_vec(LinuxFd handle, LinuxVec * ptr, u32 len) {
	return syscall3(20, (uptr)handle, (uptr)ptr, (uptr)len);
};

// |================================================================================================|
// |> Message (msghdr)                                                                              |

// [recvmsg]
i64 linux_recv_msg(LinuxFd handle, LinuxMsg * ptr, u32 flags) {
	return syscall3(47, (uptr)handle, (uptr)ptr, (uptr)flags);
};

// [sendmsg]
i64 linux_send_msg(LinuxFd handle, LinuxMsg * ptr, u32 flags) {
	return syscall3(46, (uptr)handle, (uptr)ptr, (uptr)flags);
};

// |================================================================================================|
// |> Termios (termios)                                                                             |

// [ioctl]
i64 linux_io_ctrl(LinuxFd handle, u64 request, void * arg) {
	return syscall3(16, (uptr)handle, (uptr)request, (uptr)arg);
};

b8 linux_is_tty(LinuxFd handle) {
	LinuxTermios termios;
	return linux_io_ctrl(handle, 0x5401, &termios) >= 0;
};

// [close]
i64 linux_close(LinuxFd handle) {
	return syscall1(3, (uptr)handle);
};

// |================================================================================================|
// |> Socket (sockaddr)                                                                             |

// [socket]
LinuxFd linux_socket(u32 domain, u32 type, u32 protocol) {
	return (LinuxFd)syscall3(41, (uptr)domain, (uptr)type, (uptr)protocol);
};

// [connect]
i64 linux_connect(LinuxFd handle, LinuxAddr const * addr, u32 addr_len) {
	return (i64)syscall3(42, (uptr)handle, (uptr)addr, (uptr)addr_len);
};

#endif

