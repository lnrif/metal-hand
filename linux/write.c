
#include "std/core.h"

#if LINUX && (X86_64 || AARCH64)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "std/str/core.h"
#include "linux/write.h"
#include "linux/call.h"

// |================================================================================================|
// |> Fd (fd)                                                                                       |

// [read]
i64 linux_read(LinuxFd handle, void * buf, u64 count) {
	return syscall3(LINUX_SYS_READ, (uptr)handle, (uptr)buf, (uptr)count);
};

// [write]
i64 linux_write(LinuxFd handle, void const * ptr, u64 len) {
	return syscall3(LINUX_SYS_WRITE, (uptr)handle, (uptr)ptr, (uptr)len);
};

i64 linux_write_str(LinuxFd handle, Str s) {
	return linux_write(handle, s.raw, s.len);
};

// |================================================================================================|
// |> Vec (iovec)                                                                                   |

// [readv]
i64 linux_read_vec(LinuxFd handle, LinuxVec * ptr, u32 len) {
	return syscall3(LINUX_SYS_READV, (uptr)handle, (uptr)ptr, (uptr)len);
};

// [writev]
i64 linux_write_vec(LinuxFd handle, LinuxVec * ptr, u32 len) {
	return syscall3(LINUX_SYS_WRITEV, (uptr)handle, (uptr)ptr, (uptr)len);
};

// |================================================================================================|
// |> Message (msghdr)                                                                              |

// [recvmsg]
i64 linux_recv_msg(LinuxFd handle, LinuxMsg * ptr, u32 flags) {
	return syscall3(LINUX_SYS_RECVMSG, (uptr)handle, (uptr)ptr, (uptr)flags);
};

// [sendmsg]
i64 linux_send_msg(LinuxFd handle, LinuxMsg * ptr, u32 flags) {
	return syscall3(LINUX_SYS_SENDMSG, (uptr)handle, (uptr)ptr, (uptr)flags);
};

// |================================================================================================|
// |> Termios (termios)                                                                             |

// [ioctl]
i64 linux_io_ctrl(LinuxFd handle, u64 request, void * arg) {
	return syscall3(LINUX_SYS_IOCTL, (uptr)handle, (uptr)request, (uptr)arg);
};

b8 linux_is_tty(LinuxFd handle) {
	LinuxTermios termios;
	return linux_io_ctrl(handle, 0x5401, &termios) >= 0;
};

// [close]
i64 linux_close(LinuxFd handle) {
	return syscall1(LINUX_SYS_CLOSE, (uptr)handle);
};

// |================================================================================================|
// |> Socket (sockaddr)                                                                             |

// [socket]
LinuxFd linux_socket(u32 domain, u32 type, u32 protocol) {
	return (LinuxFd)syscall3(LINUX_SYS_SOCKET, (uptr)domain, (uptr)type, (uptr)protocol);
};

// [connect]
i64 linux_connect(LinuxFd handle, LinuxAddr const * addr, u32 addr_len) {
	return (i64)syscall3(LINUX_SYS_CONNECT, (uptr)handle, (uptr)addr, (uptr)addr_len);
};

#endif

