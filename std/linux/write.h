#ifndef STD_LINUX_WRITE_H
#define STD_LINUX_WRITE_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "std/str/str.h"

// |================================================================================================|
// |> Fd (fd)                                                                                       |

typedef i32 LinuxFd;

#define LINUX_INPUT_FD  ((LinuxFd)0)
#define LINUX_OUTPUT_FD ((LinuxFd)1)
#define LINUX_ERROR_FD  ((LinuxFd)2)

// [read]
i64 linux_read(LinuxFd handle, void * buf, u64 count);
// [write]
i64 linux_write(LinuxFd handle, void const * ptr, u64 len);

// |================================================================================================|
// |> Vec (iovec)                                                                                   |

typedef union {
	Str str;
	struct {
		void const * ptr;
		u64 len;
	};
} LinuxVec;

#define VL(lit)   ((LinuxVec){.str = S(lit)})
#define VS(_str)  ((LinuxVec){.str = (_str)})
#define VZ(str_z) ((LinuxVec){.str = str_z_init(str_z).str})

// [readv]
i64 linux_read_vec(LinuxFd handle, LinuxVec * ptr, u32 len);
// [writev]
i64 linux_write_vec(LinuxFd handle, LinuxVec * ptr, u32 len);

#define linux_write_arr(handle, vecs...) \
	linux_write_vecs(handle, (LinuxVec[]){vecs}, LEN((LinuxVec[]){vecs}))

// |================================================================================================|
// |> Message (msghdr)                                                                              |

typedef struct {
	void * name; u64 name_len;
	LinuxVec * vecs; u64 vecs_len;
	void * ctrl; u64 ctrl_len; u32 flags;
} LinuxMsg;

#define LINUX_SOL_SOCKET 1
#define LINUX_SCM_RIGHTS 1

typedef struct {
	u64 len;
	i32 level;
	i32 type;
} LinuxCms;

// [recvmsg]
i64 linux_recv_msg(LinuxFd handle, LinuxMsg * ptr, u32 flags);
// [sendmsg]
i64 linux_send_msg(LinuxFd handle, LinuxMsg * ptr, u32 flags);

i64 linux_write_str(LinuxFd handle, Str s);
#define linux_write_lit(handle, lit) linux_write_str(handle, S(lit))

// |================================================================================================|
// |> Termios (termios)                                                                             |

typedef struct {
	u8 data[44];
} LinuxTermios;

// [ioctl]
i64 linux_io_ctrl(LinuxFd handle, u64 request, void * arg);
b8  linux_is_tty(LinuxFd handle);

// [close]
i64 linux_close(LinuxFd handle);

// |================================================================================================|
// |> Socket (sockaddr)                                                                             |

typedef struct {
	u16 family;
	u8 data[14];
} LinuxAddr;

typedef struct {
	u16 family;
	u8 path[108];
} LinuxAddrUnix;

////////////////////////////////
// AF - Address Family

// [AF_UNIX]
#define LINUX_AF_UNIX ((u32)1)
// [AF_INET]
#define LINUX_AF_INET ((u32)2)

////////////////////////////////
// Sock

#define LINUX_SOCK_STREAM    ((u32)1) // [SOCK_STREAM]
#define LINUX_SOCK_DGRAM     ((u32)2) // [SOCK_DGRAM]
#define LINUX_SOCK_RAW       ((u32)3) // [SOCK_RAW]
#define LINUX_SOCK_RDM       ((u32)4) // [SOCK_RDM]
#define LINUX_SOCK_SEQPACKET ((u32)5) // [SOCK_SEQPACKET]

// [socket]
LinuxFd linux_socket(u32 domain, u32 type, u32 protocol);
// [connect]
i64 linux_connect(LinuxFd handle, LinuxAddr const * addr, u32 addr_len);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_LINUX_WRITE_H
