#ifndef LINUX_FS_H
#define LINUX_FS_H

#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "linux/write.h"

#define LINUX_FS_READ_ONLY  00
#define LINUX_FS_WRITE_ONLY 01
#define LINUX_FS_READ_WRITE 02
#define LINUX_FS_CREATE     0100
#define LINUX_FS_TRUNCATE   01000

// [openat]
i64 linux_openat(LinuxFd dir, u8z const * path, u32 flags, u32 mode);
// [ftruncate]
i64 linux_ftruncate(LinuxFd fd, u64 length);
// [mkdirat]
iptr linux_mkdirat(LinuxFd dir, u8z const * path, u32 mode);

// LinuxFd linux_file_open_read(u8z const * path);
// LinuxFd linux_file_create_write(u8z const * path);
//
// u64 linux_file_read_to_buf(LinuxFd fd, void * buf, u64 max_len_without_null);
// b8  linux_file_write_from_buf(LinuxFd fd, void const * ptr, u64 len);

i64 linux_file_size(u8z const * path);

#define AT_FDCWD ((LinuxFd)-100)
#define LINUX_STATX_SIZE 0x00000200U

typedef struct {
	u64 tv_sec;
	u32 tv_nsec;
} LinuxStatxTimestamp;

typedef struct {
	u32 mask;
	u32 blksize;
	u64 attributes;
	u32 nlink;
	u32 uid;
	u32 gid;
	u16 mode;
	u16 __spare0[1];
	u64 ino;
	u64 size;
	u64 blocks;
	u64 attributes_mask;
	LinuxStatxTimestamp atime;
	LinuxStatxTimestamp btime;
	LinuxStatxTimestamp ctime;
	LinuxStatxTimestamp mtime;
	u32 rdev_major;
	u32 rdev_minor;
	u32 dev_major;
	u32 dev_minor;
	u64 mount_id;
	u64 __spare2;
	u64 __spare3[12];
} LinuxStatx;

STATIC_ASSERT(sizeof(LinuxStatx) == 256);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !LINUX_FS_H
