#include "std/core.h"

#if LINUX && X86_64
// |================================================================================================|
// |> LINUX && X86_64                                                                               |
// |================================================================================================|

#include "linux/fs.h"
#include "linux/call.h"

// [openat]
i64 linux_openat(LinuxFd dir, u8z const * path, u32 flags, u32 mode) {
	return syscall4(257, (uptr)dir, (uptr)path, (uptr)flags, (uptr)mode);
};

// [ftruncate]
i64 linux_ftruncate(LinuxFd fd, u64 length) {
	return syscall2(77, (uptr)fd, (uptr)length);
};

// [mkdirat]
iptr linux_mkdirat(LinuxFd dir, u8z const * path, u32 mode) {
	return syscall3(258, (uptr)dir, (uptr)path, (uptr)mode);
};

// LinuxFd linux_file_open_read(u8z const * path) {
// 	i64 const res = linux_openat(AT_FDCWD, path, LINUX_FS_READ_ONLY, 0);
// 	return (LinuxFd)res;
// };
//
// LinuxFd linux_file_create_write(u8z const * path) {
// 	u32 const flags = LINUX_FS_WRITE_ONLY | LINUX_FS_CREATE | LINUX_FS_TRUNCATE;
// 	u32 const mode = 0644;
// 	i64 const res = linux_openat(AT_FDCWD, path, flags, mode);
// 	return res < 0 ? (LinuxFd)-1 : (LinuxFd)res;
// };
//
// u64 linux_file_read_to_buf(LinuxFd fd, void * buf, u64 max_len_without_null) {
// 	if (fd < 0 || buf == 0 || max_len_without_null == 0) return 0;
//
// 	i64 const res = linux_read(fd, buf, max_len_without_null);
// 	if (res <= 0) {
// 		((u8 *)buf)[0] = 0;
// 		return 0;
// 	};
//
// 	u64 const read_bytes = (u64)res;
// 	((u8 *)buf)[read_bytes] = 0;
// 	return read_bytes;
// };
//
// b8 linux_file_write_from_buf(LinuxFd fd, void const * ptr, u64 len) {
// 	if (fd < 0 || (ptr == 0 && len > 0)) return false;
// 	if (len == 0) return true;
//
// 	u64 total_written = 0;
// 	while (total_written < len) {
// 		i64 const res = linux_write(
// 			fd,
// 			(u8 const *)ptr + total_written,
// 			len - total_written
// 		);
// 		if (res <= 0) return false;
// 		total_written += (u64)res;
// 	};
//
// 	return true;
// };

i64 linux_file_size(u8z const * path) {
	#define LINUX_SYS_STATX 332
	#define LINUX_STATX_SIZE 0x00000200U

	LinuxStatx st = {0};
	iptr const res = syscall5(LINUX_SYS_STATX, (uptr)AT_FDCWD, (uptr)path, 0, LINUX_STATX_SIZE, (uptr)&st);

	if (res < 0) return -1;
	if ((st.mask & LINUX_STATX_SIZE) == 0) return -1;

	return (i64)st.size;
};

#endif
